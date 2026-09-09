package main

import (
	"image"
	"image/png"
	"os"
	"path/filepath"

	"wydclient748/internal/assets"
)

func main() {
	for _, name := range []string{"loginbox2.wyt", "logo1.wyt", "logo2.wyt"} {
		texture, err := assets.LoadWYTFile(filepath.Join("assets", "current", "UI", name))
		if err != nil {
			panic(err)
		}
		img := image.NewRGBA(image.Rect(0, 0, int(texture.Width), int(texture.Height)))
		copy(img.Pix, texture.Pixels)
		out, err := os.Create(filepath.Join(".tmp_asset_preview", name+".png"))
		if err != nil {
			panic(err)
		}
		if err := png.Encode(out, img); err != nil {
			panic(err)
		}
		if err := out.Close(); err != nil {
			panic(err)
		}
	}
}
