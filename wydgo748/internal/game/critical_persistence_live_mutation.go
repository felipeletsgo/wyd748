package game

import (
	"errors"
	"log"
	"maps"
	"reflect"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// commitPlayerMutation finishes a legacy single-player mutation whose
// pre-images were captured before any change. Without a coordinator it saves
// inline and restores the pre-images on failure; with one it queues the
// detached write and publishes only after acknowledgement. It returns false
// when nothing was committed or admitted; the failure was then reported.
func (w *World) commitPlayerMutation(p *Player, images []criticalPreimage, operation, failure string,
	sidecar bool, publish func()) bool {
	failed := func() {
		if p.Session != nil && !p.Session.IsClosed() {
			p.Session.Send(wire.MessagePanel(failure))
			w.resendInventory(p)
		}
	}
	if w.criticalWrites != nil {
		if err := w.queueCriticalLiveMutation(operation, images, publish,
			criticalLiveMutationOptions{sidecar: sidecar, reject: failed}); err != nil {
			log.Printf("[#%d] %s admission rejected: %v", p.ID, operation, err)
			failed()
			return false
		}
		return true
	}
	var err error
	if sidecar {
		err = w.saveAccountAndCharStateResult(p)
	} else {
		err = w.saveAccount(p.Account)
	}
	if err != nil {
		w.restoreCriticalPreimages(images)
		log.Printf("[#%d] save %s: %v", p.ID, operation, err)
		failed()
		return false
	}
	publish()
	return true
}

// criticalPreimage is the state of one participant before a legacy producer
// mutated it. Producers capture it first, run their unchanged domain code on
// live state, then hand both images to queueCriticalLiveMutation.
type criticalPreimage struct {
	player  *Player
	account *model.Account
	coins   map[string]uint32
}

func criticalPreimages(players ...*Player) []criticalPreimage {
	images := make([]criticalPreimage, 0, len(players))
	seen := make(map[*model.Account]bool, len(players))
	for _, p := range players {
		if p == nil || p.Account == nil || seen[p.Account] {
			continue
		}
		seen[p.Account] = true
		images = append(images, criticalPreimage{player: p, account: accountStateSnapshot(p.Account), coins: maps.Clone(p.SpecialCoins)})
	}
	return images
}

// installAccountInPlace copies source into live without replacing the Chars
// backing array: Player.Char points into it and must stay valid.
func installAccountInPlace(live, source *model.Account) bool {
	if live == nil || source == nil || len(live.Chars) != len(source.Chars) {
		return false
	}
	chars := live.Chars
	*live = *source
	live.Chars = chars
	for i := range source.Chars {
		live.Chars[i] = cloneCharacterState(&source.Chars[i])
	}
	live.CelestialCapsules = make([]model.CelestialCapsule, len(source.CelestialCapsules))
	for i := range source.CelestialCapsules {
		live.CelestialCapsules[i] = source.CelestialCapsules[i]
		live.CelestialCapsules[i].Character = cloneCharacterState(&source.CelestialCapsules[i].Character)
	}
	return true
}

// criticalLiveMutationOptions selects optional parts of the atomic write.
type criticalLiveMutationOptions struct {
	// sidecar persists the first participant's character state (affects and
	// special coins) in the same transaction as the accounts.
	sidecar bool
	// character overrides the sidecar with an explicit state; a nil state
	// deletes that UID's sidecar in the same transaction (character deletion).
	character *criticalCharacterState
	// check runs at completion before installation; it validates retained
	// non-account context such as world objects the producer depends on.
	check func() error
	// reject settles non-account side effects after a certified rejection.
	reject func()
}

var errCriticalLiveMutationIdentity = errors.New("critical persistence: live mutation lost its participant identity")

// queueCriticalLiveMutation persists a mutation a legacy producer already
// applied to live state. The mutated accounts become the detached proposal,
// live state is restored to the pre-images and stays reserved, and only a
// successful identity-checked commit installs the proposal and runs publish.
// Admission failure leaves live state restored and returns the error; the
// producer then reports failure exactly as its legacy save error path did.
func (w *World) queueCriticalLiveMutation(operation string, images []criticalPreimage,
	publish func(), options criticalLiveMutationOptions) error {
	if len(images) == 0 {
		return errCriticalLiveMutationIdentity
	}
	participants := make([]*Player, len(images))
	proposals := make([]*model.Account, len(images))
	proposedCoins := make([]map[string]uint32, len(images))
	for i, image := range images {
		participants[i] = image.player
		proposals[i] = accountStateSnapshot(image.player.Account)
		proposedCoins[i] = maps.Clone(image.player.SpecialCoins)
	}
	sidecar := options.character
	if !w.criticalStoreHasSidecars() {
		// A store without character state persists the account alone, as
		// the legacy save did; it never splits an aggregate into two writes.
		sidecar, options.sidecar = nil, false
	}
	if sidecar == nil && options.sidecar {
		owner := images[0].player
		if owner.Char == nil || owner.Char.UID == "" {
			w.restoreCriticalPreimages(images)
			return errCriticalLiveMutationIdentity
		}
		sidecar = &criticalCharacterState{uid: owner.Char.UID, state: buildCharState(owner, w.now())}
	}
	w.restoreCriticalPreimages(images)
	before := make([]*model.Account, len(images))
	for i, image := range images {
		before[i] = accountStateSnapshot(image.player.Account)
	}
	check := func() error {
		for i, image := range images {
			if !reflect.DeepEqual(accountStateSnapshot(image.player.Account), before[i]) ||
				!maps.Equal(image.player.SpecialCoins, image.coins) {
				return errCriticalLiveMutationIdentity
			}
		}
		if options.check != nil {
			return options.check()
		}
		return nil
	}
	write := criticalWrite{accounts: proposals, character: sidecar}
	_, err := w.submitCriticalWriteWithRejection(write, participants, false,
		func(criticalPersistenceResult) error {
			if err := check(); err != nil {
				return err
			}
			for i, image := range images {
				if !installAccountInPlace(image.player.Account, proposals[i]) {
					return errCriticalLiveMutationIdentity
				}
				image.player.SpecialCoins = maps.Clone(proposedCoins[i])
			}
			return nil
		}, func(criticalPersistenceResult) error {
			if publish != nil {
				publish()
			}
			return nil
		}, func(result criticalPersistenceResult) error {
			if err := check(); err != nil {
				return err
			}
			if options.reject != nil {
				options.reject()
			}
			log.Printf("%s persistence rejected: %v", operation, result.err)
			return nil
		})
	return err
}

func (w *World) restoreCriticalPreimages(images []criticalPreimage) {
	for i := len(images) - 1; i >= 0; i-- {
		installAccountInPlace(images[i].player.Account, images[i].account)
		images[i].player.SpecialCoins = maps.Clone(images[i].coins)
	}
}

// criticalStoreHasSidecars reports whether the coordinator's store persists
// character state at all. Atomic capability is enforced at admission.
func (w *World) criticalStoreHasSidecars() bool {
	backend := w.criticalWrites.queue.store
	_, legacy := backend.(charStateStore)
	_, single := backend.(playerStateStore)
	_, batch := backend.(playerStateBatchStore)
	_, combined := backend.(criticalGameStateStore)
	return legacy || single || batch || combined
}
