package playerportal

import (
	"bytes"
	"encoding/base64"
	"encoding/binary"
	"encoding/json"
	"image"
	"image/color"
	"image/png"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

// nativeMarkFixture builds a ready-to-use 16x12, 24-bit BMP of 630 bytes.
func nativeMarkFixture() []byte {
	bmp := make([]byte, 630)
	bmp[0], bmp[1] = 'B', 'M'
	binary.LittleEndian.PutUint32(bmp[2:], 630)
	binary.LittleEndian.PutUint32(bmp[10:], 54)
	binary.LittleEndian.PutUint32(bmp[14:], 40)
	binary.LittleEndian.PutUint32(bmp[18:], 16)
	binary.LittleEndian.PutUint32(bmp[22:], 12)
	binary.LittleEndian.PutUint16(bmp[26:], 1)
	binary.LittleEndian.PutUint16(bmp[28:], 24)
	for i := 54; i < len(bmp); i += 3 {
		bmp[i+1] = 255
	}
	return bmp
}

func pngFixture(t *testing.T, width, height int, fill color.NRGBA) []byte {
	t.Helper()
	img := image.NewNRGBA(image.Rect(0, 0, width, height))
	for y := 0; y < height; y++ {
		for x := 0; x < width; x++ {
			img.SetNRGBA(x, y, fill)
		}
	}
	var buf bytes.Buffer
	if err := png.Encode(&buf, img); err != nil {
		t.Fatal(err)
	}
	return buf.Bytes()
}

func TestValidateGuildMarkRecognizesNativeBMP(t *testing.T) {
	good := nativeMarkFixture()
	if err := ValidateGuildMark(good); err != nil {
		t.Fatal(err)
	}
	padded := append(append([]byte(nil), good...), 0, 0)
	binary.LittleEndian.PutUint32(padded[2:], 632)
	if err := ValidateGuildMark(padded); err != nil {
		t.Fatal("the 632-byte native variant was refused")
	}
	mutate := func(at int, value uint32) []byte {
		bad := append([]byte(nil), good...)
		binary.LittleEndian.PutUint32(bad[at:], value)
		return bad
	}
	bits32 := append([]byte(nil), good...)
	binary.LittleEndian.PutUint16(bits32[28:], 32)
	for name, bad := range map[string][]byte{
		"empty": nil, "png": []byte("\x89PNG\r\n"), "wrong width": mutate(18, 32),
		"wrong height": mutate(22, 16), "32-bit": bits32, "compressed": mutate(30, 1),
		"size field": mutate(2, 640), "too long": append(append([]byte(nil), good...), make([]byte, 100)...),
	} {
		if err := ValidateGuildMark(bad); err == nil {
			t.Errorf("%s was accepted as a native mark", name)
		}
	}
}

// A native BMP is kept byte for byte; other images are converted to it.
func TestPrepareGuildMarkKeepsNativeAndConvertsOthers(t *testing.T) {
	native := nativeMarkFixture()
	kept, err := PrepareGuildMark(native)
	if err != nil || !bytes.Equal(kept, native) {
		t.Fatal("a ready-to-use BMP was altered")
	}
	bmp, err := PrepareGuildMark(pngFixture(t, 32, 24, color.NRGBA{R: 200, G: 100, B: 50, A: 255}))
	if err != nil || ValidateGuildMark(bmp) != nil {
		t.Fatalf("converted mark is not native: %v", err)
	}
	if bmp[54] != 50 || bmp[55] != 100 || bmp[56] != 200 {
		t.Fatalf("first pixel BGR = %d,%d,%d", bmp[54], bmp[55], bmp[56])
	}
	for _, bad := range [][]byte{nil, []byte("not an image"), bytes.Repeat([]byte{1}, maxGuildMarkInput+1)} {
		if _, err := PrepareGuildMark(bad); err == nil {
			t.Fatalf("accepted %d bytes of invalid input", len(bad))
		}
	}
}

func TestGuildLeaderManagesGuildMark(t *testing.T) {
	h, b := testHandler(t)
	h.cfg.GuildMarkDir = t.TempDir()
	b.guildSnapshot = publicGuildFixture()
	b.characters = map[string][]Character{
		"owner":  {{Slot: 0, Name: "Leader"}},
		"player": {{Slot: 0, Name: "Member"}},
	}
	upload := `{"image":"` + base64.StdEncoding.EncodeToString(nativeMarkFixture()) + `"}`

	if w := request(h, "GET", "/api/player/v1/guild", "", nil, ""); w.Code != 401 {
		t.Fatalf("anonymous guild access = %d", w.Code)
	}
	member, memberSession := authenticatedSession(t, h, "Player")
	if w := request(h, "POST", "/api/player/v1/guild/mark", upload, member, memberSession.CSRF); w.Code != 403 {
		t.Fatalf("member upload = %d %s", w.Code, w.Body.String())
	}

	leader, leaderSession := authenticatedSession(t, h, "Owner")
	if w := request(h, "POST", "/api/player/v1/guild/mark", upload, leader, leaderSession.CSRF); w.Code != 200 {
		t.Fatalf("leader upload = %d %s", w.Code, w.Body.String())
	}
	stored, err := os.ReadFile(filepath.Join(h.cfg.GuildMarkDir, "0001.bmp"))
	if err != nil || !bytes.Equal(stored, nativeMarkFixture()) {
		t.Fatalf("stored mark differs from the leader's BMP: %v", err)
	}

	w := request(h, "GET", "/api/player/v1/guild", "", leader, "")
	var manage struct {
		Guild  struct{ Name string }
		Leader bool
		Mark   struct{ URL string }
	}
	if err := json.Unmarshal(w.Body.Bytes(), &manage); err != nil || w.Code != 200 ||
		manage.Guild.Name != "Alpha Team" || !manage.Leader || manage.Mark.URL != "/patch/guildmark/b00000001.bmp" {
		t.Fatalf("guild management view = %d %s", w.Code, w.Body.String())
	}

	if w := request(h, "POST", "/api/player/v1/guild/mark", `{"image":"`+base64.StdEncoding.EncodeToString([]byte("junk"))+`"}`,
		leader, leaderSession.CSRF); w.Code != 400 || !strings.Contains(w.Body.String(), "BMP") {
		t.Fatalf("invalid image = %d %s", w.Code, w.Body.String())
	}
	if w := request(h, "POST", "/api/player/v1/guild/mark/delete", `{}`, leader, leaderSession.CSRF); w.Code != 200 {
		t.Fatalf("delete = %d %s", w.Code, w.Body.String())
	}
	if _, err := os.Stat(filepath.Join(h.cfg.GuildMarkDir, "0001.bmp")); !os.IsNotExist(err) {
		t.Fatal("the mark was not removed")
	}
}
