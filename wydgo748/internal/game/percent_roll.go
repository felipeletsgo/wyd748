package game

import "fmt"

type percentRoll struct {
	Roll    int
	Chance  int
	Success bool
}

func (w *World) rollPercent(chance int) percentRoll {
	chance = clampInt(chance, 0, 100)
	roll := w.intn(100) + 1
	return percentRoll{
		Roll:    roll,
		Chance:  chance,
		Success: roll <= chance,
	}
}

func (r percentRoll) message() string {
	status := "Fail"
	if r.Success {
		status = "Success"
	}
	return fmt.Sprintf("%s: %d/%d", status, r.Roll, r.Chance)
}

func (r percentRoll) namedMessage(name string) string {
	if name == "" {
		return r.message()
	}
	return fmt.Sprintf("%s %s", name, r.message())
}
