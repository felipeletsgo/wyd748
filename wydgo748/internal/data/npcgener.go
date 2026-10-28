package data

import (
	"bufio"
	"fmt"
	"log"
	"os"
	"strconv"
	"strings"

	"wydgo/internal/model"
)

// LoadNPCGener reads the traditional TMSrv generator format. Effective indices
// follow section order; bracketed labels are decorative. #* disables a section.
func LoadNPCGener(path string) ([]model.NPCGener, error) {
	return loadNPCGener(path, false)
}

// LoadChannelNPCGener preserves source records but disables routes outside the
// installed terrain. Disabled records retain their original index and values.
// The caller must report these reservations; coordinates are never relocated.
func LoadChannelNPCGener(path string) ([]model.NPCGener, error) {
	return loadNPCGener(path, true)
}

func loadNPCGener(path string, disableOutsideTerrain bool) ([]model.NPCGener, error) {
	f, err := os.Open(path)
	if err != nil {
		return nil, err
	}
	defer f.Close()

	var out []model.NPCGener
	var cur *model.NPCGener
	scanner := bufio.NewScanner(f)
	scanner.Buffer(make([]byte, 1024), 1024*1024)
	lineNo := 0
	for scanner.Scan() {
		lineNo++
		line := strings.TrimSpace(scanner.Text())
		if line == "" || strings.HasPrefix(line, "//") {
			continue
		}
		if strings.HasPrefix(line, "#") {
			out = append(out, model.NPCGener{
				Index: len(out), Label: generatorLabel(line), Enabled: !strings.HasPrefix(line, "#*"),
				MinuteGenerate: -1, MaxNumMob: 1,
			})
			cur = &out[len(out)-1]
			continue
		}
		if cur == nil {
			continue // TMSrv also ignores text before the first section.
		}
		fields := strings.Fields(line)
		if len(fields) < 2 {
			continue
		}
		key, value := fields[0], fields[1]
		if err := setGenerFieldValue(cur, key, value, !disableOutsideTerrain); err != nil {
			return nil, fmt.Errorf("NPCGener %s:%d: %w", path, lineNo, err)
		}
	}
	if err := scanner.Err(); err != nil {
		return nil, err
	}
	for i := range out {
		g := &out[i]
		if !g.Enabled {
			continue
		}
		if disableOutsideTerrain {
			for _, point := range g.Segments {
				if int(point.X) >= model.TerrainWidth || int(point.Y) >= model.TerrainHeight {
					log.Printf("NPCGener %s: disabled group %d (%s): route (%d,%d) outside installed terrain", path, g.Index, g.Leader, point.X, point.Y)
					g.Enabled = false
					break
				}
			}
			if !g.Enabled {
				continue
			}
		}
		if g.Leader == "" || g.Leader == "0" {
			return nil, fmt.Errorf("NPCGener %s: group %d has no valid Leader", path, g.Index)
		}
		if g.Follower == "" || g.Follower == "0" {
			g.Follower = g.Leader
		}
		if g.MaxGroup < g.MinGroup {
			// Hand-edited files may put the ceiling below the floor. Preserve
			// boot compatibility by normalizing to a fixed-size group.
			g.MaxGroup = g.MinGroup
		}
		if g.MaxNumMob <= 0 {
			if disableOutsideTerrain {
				log.Printf("NPCGener %s: disabled group %d (%s): nonpositive capacity %d", path, g.Index, g.Leader, g.MaxNumMob)
				g.Enabled = false // Nonpositive capacity cannot admit a live group.
				continue
			}
			return nil, fmt.Errorf("NPCGener %s: group %d MaxNumMob must be positive", path, g.Index)
		}
	}
	return out, nil
}

func setGenerField(g *model.NPCGener, key, value string) error {
	return setGenerFieldValue(g, key, value, true)
}

func setGenerFieldValue(g *model.NPCGener, key, value string, strictTerrain bool) error {
	switch key {
	case "Leader:":
		g.Leader = value
		return nil
	case "Follower:":
		g.Follower = value
		return nil
	}
	if strings.HasSuffix(key, "Action:") {
		if idx, ok := generSegmentIndex(key); ok {
			g.Segments[idx].Action = value
		}
		return nil
	}

	// Ignore unknown keys before integer parsing, including malformed comments.
	// Recognized keys with malformed values remain errors.
	seg, isSeg := generSegmentIndex(key)
	isSegInt := isSeg && (strings.HasSuffix(key, "X:") || strings.HasSuffix(key, "Y:") ||
		strings.HasSuffix(key, "Range:") || strings.HasSuffix(key, "Wait:"))
	switch key {
	case "MinuteGenerate:", "MaxNumMob:", "MinGroup:", "MaxGroup:", "RouteType:", "Formation:":
	default:
		if !isSegInt {
			return nil // Ignore unknown lines.
		}
	}

	n, err := strconv.Atoi(value)
	if err != nil {
		return fmt.Errorf("%s expects an integer, received %q", key, value)
	}
	switch key {
	case "MinuteGenerate:":
		g.MinuteGenerate = n
	case "MaxNumMob:":
		g.MaxNumMob = n
	case "MinGroup:":
		g.MinGroup = n
	case "MaxGroup:":
		g.MaxGroup = n
	case "RouteType:":
		g.RouteType = n
	case "Formation:":
		g.Formation = n
	default: // Integer segment field; seg was resolved above.
		switch {
		case strings.HasSuffix(key, "X:"):
			if n <= 0 || n > 65535 || strictTerrain && n >= model.TerrainWidth {
				return fmt.Errorf("%s outside supported coordinates: %d", key, n)
			}
			g.Segments[seg].X = uint16(n)
		case strings.HasSuffix(key, "Y:"):
			if n <= 0 || n > 65535 || strictTerrain && n >= model.TerrainHeight {
				return fmt.Errorf("%s outside supported coordinates: %d", key, n)
			}
			g.Segments[seg].Y = uint16(n)
		case strings.HasSuffix(key, "Range:"):
			if n < 0 || n > 65535 {
				return fmt.Errorf("%s invalid value: %d", key, n)
			}
			g.Segments[seg].Range = uint16(n)
		case strings.HasSuffix(key, "Wait:"):
			g.Segments[seg].Wait = n
		}
	}
	return nil
}

func generSegmentIndex(key string) (int, bool) {
	switch {
	case strings.HasPrefix(key, "Start"):
		return 0, true
	case strings.HasPrefix(key, "Segment1"):
		return 1, true
	case strings.HasPrefix(key, "Segment2"):
		return 2, true
	case strings.HasPrefix(key, "Segment3"):
		return 3, true
	case strings.HasPrefix(key, "Dest"):
		return 4, true
	default:
		return 0, false
	}
}

// generatorLabel reads the number of a "#	[n]" header; zero when absent.
func generatorLabel(line string) int {
	open := strings.IndexByte(line, '[')
	end := strings.IndexByte(line, ']')
	if open < 0 || end <= open+1 {
		return 0
	}
	n, err := strconv.Atoi(strings.TrimSpace(line[open+1 : end]))
	if err != nil {
		return 0
	}
	return n
}
