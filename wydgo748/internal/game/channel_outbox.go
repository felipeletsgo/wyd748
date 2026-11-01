package game

import (
	"context"
	"crypto/rand"
	"encoding/hex"
	"encoding/json"
	"expvar"
	"log"
	"strings"
	"sync"
	"time"

	"wydgo/internal/store"
	"wydgo/internal/wire"
)

// Cross-process channel messages (DOCS/server/server-correction-plan.md, W9 phase 4).
// When channels run in separate processes, chat, staff notices and sanction
// changes that address other channels go through the channel_messages table.
// The actor queues each message without waiting; one worker stores batches,
// and one reader delivers the messages of other processes to the World.
// Chat is best effort: a full queue or a database failure drops the message
// and counts it.

type channelMessageStore interface {
	PublishChannelMessages([]store.ChannelMessage) error
	ChannelMessageStart() (time.Time, error)
	ReadChannelMessages(self string, channel byte, lastID int64, since time.Time, limit int) ([]store.ChannelMessage, error)
	CleanupChannelMessages() error
	ListenChannelMessages(ctx context.Context, wake func())
}

const (
	channelOutboxCapacity = 1024
	channelOutboxBatch    = 100
	channelOutboxPoll     = time.Second
	channelOutboxCleanup  = time.Minute
	channelOutboxSeenTTL  = 2 * time.Minute
)

var metricChannelOutbox = expvar.NewMap("world_channel_outbox_total")

// Message kinds.
const (
	outboxGlobalChat      = "chat.global"
	outboxCitizenshipChat = "chat.citizenship"
	outboxGuildChat       = "chat.guild"
	outboxWhisper         = "chat.whisper"
	outboxStaffNotice     = "staff.notice"
	outboxSanction        = "staff.sanction"
)

// outboxPayload is the JSON body of every message kind.
type outboxPayload struct {
	Sender      string `json:"sender,omitempty"`
	Target      string `json:"target,omitempty"`
	Message     string `json:"message,omitempty"`
	Citizenship byte   `json:"citizenship,omitempty"`
	GuildID     uint16 `json:"guildId,omitempty"`
	Account     string `json:"account,omitempty"`
	Notify      string `json:"notify,omitempty"`
	// Request and ReplyTo identify a request that expects replies;
	// Body carries a request or reply value; Delivered answers a whisper.
	Request   string          `json:"request,omitempty"`
	ReplyTo   byte            `json:"replyTo,omitempty"`
	Body      json.RawMessage `json:"body,omitempty"`
	Delivered bool            `json:"delivered,omitempty"`
}

const (
	outboxReply = "reply"
	// outboxRequestTimeout bounds the wait for the replies of a request.
	outboxRequestTimeout = 3 * time.Second
)

// ChannelOutbox is one process's message client for its single channel.
type ChannelOutbox struct {
	store   channelMessageStore
	process string
	channel byte
	queue   chan store.ChannelMessage
	mu      sync.Mutex
	pending map[string]chan outboxPayload
}

// NewChannelOutbox returns nil when the store cannot hold channel messages.
func NewChannelOutbox(backend store.Store, process string, channel byte) *ChannelOutbox {
	messages, ok := backend.(channelMessageStore)
	if !ok || process == "" {
		return nil
	}
	return &ChannelOutbox{store: messages, process: process, channel: channel,
		queue: make(chan store.ChannelMessage, channelOutboxCapacity)}
}

// WithChannelOutbox sends messages for other channels through the outbox.
func WithChannelOutbox(outbox *ChannelOutbox) WorldOption {
	return func(w *World) { w.outbox = outbox }
}

// publish queues one message for every channel without blocking the actor.
func (o *ChannelOutbox) publish(kind string, payload outboxPayload) {
	o.publishTo(0, kind, payload)
}

// publishTo queues one message for one channel, or every channel for 0.
func (o *ChannelOutbox) publishTo(target byte, kind string, payload outboxPayload) {
	if o == nil {
		return
	}
	body, err := json.Marshal(payload)
	if err != nil {
		return
	}
	select {
	case o.queue <- store.ChannelMessage{Source: o.process, Target: target, Kind: kind, Payload: body}:
		metricChannelOutbox.Add("queued", 1)
	default:
		metricChannelOutbox.Add("dropped_full", 1)
	}
}

// request sends a request to every other channel and returns the replies
// that arrive before expect replies, the timeout or ctx.
func (o *ChannelOutbox) request(ctx context.Context, kind string, payload outboxPayload, expect int) []outboxPayload {
	if o == nil || expect <= 0 {
		return nil
	}
	var id [12]byte
	if _, err := rand.Read(id[:]); err != nil {
		return nil
	}
	payload.Request, payload.ReplyTo = hex.EncodeToString(id[:]), o.channel
	replies := make(chan outboxPayload, expect)
	o.mu.Lock()
	if o.pending == nil {
		o.pending = make(map[string]chan outboxPayload)
	}
	o.pending[payload.Request] = replies
	o.mu.Unlock()
	defer func() {
		o.mu.Lock()
		delete(o.pending, payload.Request)
		o.mu.Unlock()
	}()
	o.publishTo(0, kind, payload)
	timeout := time.NewTimer(outboxRequestTimeout)
	defer timeout.Stop()
	var collected []outboxPayload
	for len(collected) < expect {
		select {
		case reply := <-replies:
			collected = append(collected, reply)
		case <-timeout.C:
			metricChannelOutbox.Add("request_timeout", 1)
			return collected
		case <-ctx.Done():
			return collected
		}
	}
	return collected
}

// reply answers a request of another process.
func (o *ChannelOutbox) reply(request outboxPayload, answer outboxPayload) {
	if o == nil || request.Request == "" || request.ReplyTo == 0 {
		return
	}
	answer.Request = request.Request
	o.publishTo(request.ReplyTo, outboxReply, answer)
}

// deliverReply hands a reply to the waiting request, if any.
func (o *ChannelOutbox) deliverReply(raw []byte) {
	var reply outboxPayload
	if json.Unmarshal(raw, &reply) != nil {
		return
	}
	o.mu.Lock()
	waiting := o.pending[reply.Request]
	o.mu.Unlock()
	if waiting == nil {
		return
	}
	select {
	case waiting <- reply:
	default:
	}
}

// Run stores queued messages and delivers other processes' messages to w
// until ctx ends.
func (o *ChannelOutbox) Run(ctx context.Context, w *World) {
	if o == nil {
		return
	}
	go o.runPublisher(ctx)
	o.runReader(ctx, w)
}

func (o *ChannelOutbox) runPublisher(ctx context.Context) {
	for {
		var batch []store.ChannelMessage
		select {
		case <-ctx.Done():
			return
		case message := <-o.queue:
			batch = append(batch, message)
		}
	fill:
		for len(batch) < channelOutboxBatch {
			select {
			case message := <-o.queue:
				batch = append(batch, message)
			default:
				break fill
			}
		}
		if err := o.store.PublishChannelMessages(batch); err != nil {
			metricChannelOutbox.Add("dropped_store", int64(len(batch)))
			log.Printf("channel outbox: store %d message(s) failed: %v", len(batch), err)
			continue
		}
		metricChannelOutbox.Add("published", int64(len(batch)))
	}
}

func (o *ChannelOutbox) runReader(ctx context.Context, w *World) {
	since, err := o.store.ChannelMessageStart()
	for err != nil {
		log.Printf("channel outbox: start failed: %v", err)
		select {
		case <-ctx.Done():
			return
		case <-time.After(2 * time.Second):
		}
		since, err = o.store.ChannelMessageStart()
	}
	wake := make(chan struct{}, 1)
	go o.store.ListenChannelMessages(ctx, func() {
		select {
		case wake <- struct{}{}:
		default:
		}
	})
	poll := time.NewTicker(channelOutboxPoll)
	defer poll.Stop()
	cleanup := time.Now().Add(channelOutboxCleanup)
	seen := make(map[int64]time.Time)
	var lastID int64
	for {
		select {
		case <-ctx.Done():
			return
		case <-wake:
		case <-poll.C:
		}
		messages, err := o.store.ReadChannelMessages(o.process, o.channel, lastID, since, 500)
		if err != nil {
			log.Printf("channel outbox: read failed: %v", err)
			continue
		}
		now := time.Now()
		for _, message := range messages {
			if message.ID > lastID {
				lastID = message.ID
			}
			if _, done := seen[message.ID]; done {
				continue
			}
			seen[message.ID] = now
			metricChannelOutbox.Add("delivered", 1)
			if message.Kind == outboxReply {
				o.deliverReply(message.Payload)
				continue
			}
			w.commands <- command{queuedAt: now, channelMessage: &message}
		}
		for id, at := range seen {
			if now.Sub(at) > channelOutboxSeenTTL {
				delete(seen, id)
			}
		}
		if now.After(cleanup) {
			cleanup = now.Add(channelOutboxCleanup)
			if err := o.store.CleanupChannelMessages(); err != nil {
				log.Printf("channel outbox: cleanup failed: %v", err)
			}
		}
	}
}

// onChannelMessage delivers another process's message to this channel.
func (w *World) onChannelMessage(message *store.ChannelMessage) {
	var payload outboxPayload
	if err := json.Unmarshal(message.Payload, &payload); err != nil {
		metricChannelOutbox.Add("invalid", 1)
		return
	}
	switch message.Kind {
	case outboxGlobalChat, outboxCitizenshipChat:
		for _, observer := range w.players {
			if observer == nil || !observer.InWorld || observer.Session == nil || observer.Char == nil ||
				message.Kind == outboxCitizenshipChat && observer.Char.Citizenship != payload.Citizenship {
				continue
			}
			observer.Session.Send(wire.MessageWhisper(0, payload.Sender, payload.Message, 3))
		}
	case outboxGuildChat:
		guild := w.guilds.FindByID(payload.GuildID)
		if guild == nil {
			return
		}
		for _, member := range w.players {
			if member == nil || !member.InWorld || member.Session == nil || member.Char == nil ||
				member.Char.GuildID != payload.GuildID || guild.Member(member.Char.Name) == nil {
				continue
			}
			member.Session.Send(wire.MessageWhisper(0, payload.Sender, payload.Message, 3))
		}
	case outboxWhisper:
		recipient := w.playerByCharacterName(payload.Target)
		if recipient == nil || recipient.Session == nil {
			w.outbox.reply(payload, outboxPayload{})
			return
		}
		recipient.Session.Send(wire.MessageWhisper(0, payload.Sender, payload.Message, 0))
		w.outbox.reply(payload, outboxPayload{Delivered: true})
		if !strings.HasPrefix(payload.Message, "!") {
			log.Printf("WHISPER %q -> %q (from another channel)", payload.Sender, recipient.Char.Name)
		}
	case outboxStaffNotice:
		w.staffNotice(func(*World) string { return payload.Message })
	case outboxSanction:
		backend, ok := w.sanctionBackend()
		if !ok {
			return
		}
		account, notify := payload.Account, payload.Notify
		go func() {
			ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
			defer cancel()
			w.pushSanctionState(ctx, backend, account, notify)
		}()
	case outboxStaffOverview, outboxStaffPresence, outboxStaffKick, outboxStaffTeleport:
		w.onRemoteStaffRequest(message.Kind, payload)
	default:
		metricChannelOutbox.Add("unknown_kind", 1)
	}
}
