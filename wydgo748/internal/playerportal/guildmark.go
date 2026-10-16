package playerportal

import (
	"bytes"
	"encoding/base64"
	"encoding/binary"
	"errors"
	"fmt"
	"image"
	"image/color"
	_ "image/gif"  // GIF uploads
	_ "image/jpeg" // JPEG uploads
	_ "image/png"  // PNG uploads
	"net/http"
	"os"
	"path/filepath"
	"strings"
	"time"

	"wydgo/internal/model"
)

// The WYD 7.48 client draws a guild mark from a 16x12, 24-bit BMP of 630 or
// 632 bytes (TMFieldScene::Guildmark_IsCorrectBMP). A leader's ready-to-use
// BMP is stored unchanged; other images are converted. The client requests
// "b<group:02><channel:02><guild:04>.bmp"; Caddy maps every group/channel to
// the single "<guild:04>.bmp" file written here.
const (
	GuildMarkWidth   = 16
	GuildMarkHeight  = 12
	guildMarkBMPSize = 54 + GuildMarkWidth*GuildMarkHeight*3
	guildMarkURLBase = "/patch/guildmark/"
	// maxGuildMarkInput bounds an image that the server converts.
	maxGuildMarkInput = 64 << 10
)

var errGuildMarkImage = errors.New("use a 16x12 24-bit BMP, or a PNG, JPEG, GIF or BMP image up to 64 KB")

// GuildMarkFileName is the stored mark of one guild.
func GuildMarkFileName(guild uint16) string { return fmt.Sprintf("%04d.bmp", guild) }

// GuildMarkURL is the public address of a stored mark, in the client format.
func GuildMarkURL(guild uint16) string {
	return fmt.Sprintf("%sb0000%04d.bmp", guildMarkURLBase, guild)
}

// ValidateGuildMark reports whether raw is already a ready-to-use native mark:
// the exact BMP that TMFieldScene::Guildmark_IsCorrectBMP and the 632-byte
// texture loader accept (BM, 630/632 bytes, 54-byte BI_RGB header, 16x12,
// 24 bits).
func ValidateGuildMark(raw []byte) error {
	if len(raw) != guildMarkBMPSize && len(raw) != guildMarkBMPSize+2 {
		return errGuildMarkImage
	}
	if raw[0] != 'B' || raw[1] != 'M' ||
		int(binary.LittleEndian.Uint32(raw[2:])) != len(raw) ||
		binary.LittleEndian.Uint32(raw[10:]) != 54 ||
		binary.LittleEndian.Uint32(raw[14:]) != 40 ||
		int32(binary.LittleEndian.Uint32(raw[18:])) != GuildMarkWidth ||
		int32(binary.LittleEndian.Uint32(raw[22:])) != GuildMarkHeight ||
		binary.LittleEndian.Uint16(raw[26:]) != 1 ||
		binary.LittleEndian.Uint16(raw[28:]) != 24 ||
		binary.LittleEndian.Uint32(raw[30:]) != 0 {
		return errGuildMarkImage
	}
	return nil
}

// PrepareGuildMark keeps a ready-to-use native BMP unchanged and converts any
// other supported image into it.
func PrepareGuildMark(raw []byte) ([]byte, error) {
	if ValidateGuildMark(raw) == nil {
		return raw, nil
	}
	return EncodeGuildMark(raw)
}

// EncodeGuildMark converts a PNG, JPEG, GIF or uncompressed BMP into the
// native 16x12 24-bit BMP, averaging source pixels per target pixel
// (transparent pixels become black, the client's background).
func EncodeGuildMark(raw []byte) ([]byte, error) {
	if len(raw) == 0 || len(raw) > maxGuildMarkInput {
		return nil, errGuildMarkImage
	}
	src, err := decodeGuildMarkImage(raw)
	if err != nil {
		return nil, errGuildMarkImage
	}
	bounds := src.Bounds()
	if bounds.Dx() <= 0 || bounds.Dy() <= 0 || bounds.Dx() > 1024 || bounds.Dy() > 1024 {
		return nil, errGuildMarkImage
	}
	out := make([]byte, guildMarkBMPSize)
	out[0], out[1] = 'B', 'M'
	binary.LittleEndian.PutUint32(out[2:], guildMarkBMPSize)
	binary.LittleEndian.PutUint32(out[10:], 54)
	binary.LittleEndian.PutUint32(out[14:], 40)
	binary.LittleEndian.PutUint32(out[18:], GuildMarkWidth)
	binary.LittleEndian.PutUint32(out[22:], GuildMarkHeight)
	binary.LittleEndian.PutUint16(out[26:], 1)
	binary.LittleEndian.PutUint16(out[28:], 24)
	binary.LittleEndian.PutUint32(out[34:], GuildMarkWidth*GuildMarkHeight*3)
	for ty := 0; ty < GuildMarkHeight; ty++ {
		y0 := bounds.Min.Y + ty*bounds.Dy()/GuildMarkHeight
		y1 := bounds.Min.Y + (ty+1)*bounds.Dy()/GuildMarkHeight
		if y1 <= y0 {
			y1 = y0 + 1
		}
		for tx := 0; tx < GuildMarkWidth; tx++ {
			x0 := bounds.Min.X + tx*bounds.Dx()/GuildMarkWidth
			x1 := bounds.Min.X + (tx+1)*bounds.Dx()/GuildMarkWidth
			if x1 <= x0 {
				x1 = x0 + 1
			}
			var r, g, b, n uint64
			for y := y0; y < y1; y++ {
				for x := x0; x < x1; x++ {
					c := color.NRGBA64Model.Convert(src.At(x, y)).(color.NRGBA64)
					a := uint64(c.A)
					r += uint64(c.R) * a / 0xFFFF
					g += uint64(c.G) * a / 0xFFFF
					b += uint64(c.B) * a / 0xFFFF
					n++
				}
			}
			// BMP rows are bottom-up and BGR; 48 bytes per row need no padding.
			at := 54 + ((GuildMarkHeight-1-ty)*GuildMarkWidth+tx)*3
			out[at] = byte(b / n >> 8)
			out[at+1] = byte(g / n >> 8)
			out[at+2] = byte(r / n >> 8)
		}
	}
	return out, nil
}

func decodeGuildMarkImage(raw []byte) (image.Image, error) {
	if len(raw) >= 2 && raw[0] == 'B' && raw[1] == 'M' {
		return decodeUncompressedBMP(raw)
	}
	img, _, err := image.Decode(bytes.NewReader(raw))
	return img, err
}

// decodeUncompressedBMP reads 24- or 32-bit BI_RGB bitmaps, the formats image
// editors save for small icons.
func decodeUncompressedBMP(raw []byte) (image.Image, error) {
	if len(raw) < 54 {
		return nil, errGuildMarkImage
	}
	offset := int(binary.LittleEndian.Uint32(raw[10:]))
	width := int(int32(binary.LittleEndian.Uint32(raw[18:])))
	height := int(int32(binary.LittleEndian.Uint32(raw[22:])))
	bits := int(binary.LittleEndian.Uint16(raw[28:]))
	compression := binary.LittleEndian.Uint32(raw[30:])
	topDown := height < 0
	if topDown {
		height = -height
	}
	if width <= 0 || height <= 0 || width > 1024 || height > 1024 || compression != 0 || (bits != 24 && bits != 32) {
		return nil, errGuildMarkImage
	}
	stride := (width*bits/8 + 3) &^ 3
	if offset < 54 || offset+stride*height > len(raw) {
		return nil, errGuildMarkImage
	}
	img := image.NewNRGBA(image.Rect(0, 0, width, height))
	for y := 0; y < height; y++ {
		row := y
		if !topDown {
			row = height - 1 - y
		}
		base := offset + row*stride
		for x := 0; x < width; x++ {
			p := base + x*bits/8
			img.SetNRGBA(x, y, color.NRGBA{R: raw[p+2], G: raw[p+1], B: raw[p], A: 0xFF})
		}
	}
	return img, nil
}

// guildMembership is the guild an account belongs to through one of its
// characters, preferring the character that leads it.
type guildMembership struct {
	Guild     SavedPublicGuild
	Character string
	Rank      byte
}

func accountGuild(profile Profile, snapshot PublicGuildSnapshot) (guildMembership, bool) {
	names := make(map[string]string, len(profile.Characters))
	for _, character := range profile.Characters {
		if character.Name != "" {
			names[strings.ToLower(character.Name)] = character.Name
		}
	}
	var found guildMembership
	ok := false
	for _, guild := range snapshot.Guilds {
		for _, member := range guild.Members {
			name, mine := names[strings.ToLower(member.Name)]
			if !mine {
				continue
			}
			if !ok || member.Rank == model.GuildRankLeader && found.Rank != model.GuildRankLeader {
				found, ok = guildMembership{Guild: guild, Character: name, Rank: member.Rank}, true
			}
		}
	}
	return found, ok
}

// writeGuildMark stores the mark atomically so the web server never serves
// a partially written file.
func writeGuildMark(dir string, guild uint16, bmp []byte) error {
	temp, err := os.CreateTemp(dir, ".mark-*")
	if err != nil {
		return err
	}
	name := temp.Name()
	defer os.Remove(name)
	if _, err := temp.Write(bmp); err != nil {
		temp.Close()
		return err
	}
	if err := temp.Chmod(0o644); err != nil {
		temp.Close()
		return err
	}
	if err := temp.Close(); err != nil {
		return err
	}
	return os.Rename(name, filepath.Join(dir, GuildMarkFileName(guild)))
}

func guildMarkInfo(dir string, guild uint16) map[string]any {
	if dir == "" {
		return nil
	}
	stat, err := os.Stat(filepath.Join(dir, GuildMarkFileName(guild)))
	if err != nil {
		return nil
	}
	return map[string]any{"url": GuildMarkURL(guild), "updatedAt": stat.ModTime().UTC().Format(time.RFC3339)}
}

// guildLeadership resolves the signed-in account's guild. Only the leader may
// change the mark; members can still see it.
func (h *Handler) guildLeadership(w http.ResponseWriter, r *http.Request) (guildMembership, bool) {
	s, _, ok := h.current(r)
	if !ok || s.Account == "" {
		failure(w, 401, "Sign in to manage your guild")
		return guildMembership{}, false
	}
	profile, err := h.cfg.Backend.Profile(r.Context(), s.Account)
	if err != nil {
		failure(w, 503, "Profile unavailable")
		return guildMembership{}, false
	}
	snapshot, err := h.publicGuildSnapshot(r.Context())
	if err != nil {
		failure(w, 503, "Guild unavailable")
		return guildMembership{}, false
	}
	membership, found := accountGuild(profile, snapshot)
	if !found {
		failure(w, 404, "None of your characters belongs to a guild")
		return guildMembership{}, false
	}
	return membership, true
}

func (h *Handler) guildManage(w http.ResponseWriter, r *http.Request) {
	membership, ok := h.guildLeadership(w, r)
	if !ok {
		return
	}
	reply(w, 200, map[string]any{"version": 1, "guild": map[string]any{
		"id": membership.Guild.ID, "name": membership.Guild.Name, "kingdom": membership.Guild.Kingdom,
		"fame": membership.Guild.Fame, "members": membership.Guild.Members,
	}, "character": membership.Character, "rank": membership.Rank,
		"leader": membership.Rank == model.GuildRankLeader, "marksEnabled": h.cfg.GuildMarkDir != "",
		"mark": guildMarkInfo(h.cfg.GuildMarkDir, membership.Guild.ID), "source": "persisted snapshot"})
}

func (h *Handler) uploadGuildMark(w http.ResponseWriter, r *http.Request) {
	var body struct {
		Image string `json:"image"`
	}
	// Base64 of the 64 KB image limit plus JSON framing.
	if !decodeLimited(w, r, &body, 96<<10) {
		return
	}
	membership, ok := h.guildLeadership(w, r)
	if !ok {
		return
	}
	if membership.Rank != model.GuildRankLeader {
		failure(w, 403, "Only the guild leader can change the guild mark")
		return
	}
	if h.cfg.GuildMarkDir == "" {
		failure(w, 503, "Guild marks are not available")
		return
	}
	if !h.allow(fmt.Sprintf("guildmark:%d", membership.Guild.ID), 6) {
		failure(w, 429, "Please wait before changing the mark again")
		return
	}
	raw, err := base64.StdEncoding.DecodeString(body.Image)
	if err != nil {
		failure(w, 400, errGuildMarkImage.Error())
		return
	}
	bmp, err := PrepareGuildMark(raw)
	if err != nil {
		failure(w, 400, err.Error())
		return
	}
	if err := writeGuildMark(h.cfg.GuildMarkDir, membership.Guild.ID, bmp); err != nil {
		failure(w, 503, "The guild mark could not be saved")
		return
	}
	reply(w, 200, map[string]any{"version": 1, "mark": guildMarkInfo(h.cfg.GuildMarkDir, membership.Guild.ID)})
}

func (h *Handler) deleteGuildMark(w http.ResponseWriter, r *http.Request) {
	if !decode(w, r, &struct{}{}) {
		return
	}
	membership, ok := h.guildLeadership(w, r)
	if !ok {
		return
	}
	if membership.Rank != model.GuildRankLeader {
		failure(w, 403, "Only the guild leader can change the guild mark")
		return
	}
	if h.cfg.GuildMarkDir == "" {
		failure(w, 503, "Guild marks are not available")
		return
	}
	err := os.Remove(filepath.Join(h.cfg.GuildMarkDir, GuildMarkFileName(membership.Guild.ID)))
	if err != nil && !errors.Is(err, os.ErrNotExist) {
		failure(w, 503, "The guild mark could not be removed")
		return
	}
	reply(w, 200, map[string]any{"version": 1, "mark": nil})
}
