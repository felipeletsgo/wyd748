package game

import (
	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// queueCargoGoldTransfer is called after authoritative banker/balance validation
// on World. Workers never see the live account, player or continuation. This
// producer stays staged until NewWorld enables the full persistence migration.
func (w *World) queueCargoGoldTransfer(p *Player, deposit bool, amount uint32) error {
	if p.CharSlot < 0 || p.CharSlot >= len(p.Account.Chars) || p.Char != &p.Account.Chars[p.CharSlot] {
		return errCriticalPersistenceIdentity
	}
	proposed := accountStateSnapshot(p.Account)
	character := &proposed.Chars[p.CharSlot]
	if deposit {
		character.Gold -= amount
		proposed.CargoGold += amount
	} else {
		proposed.CargoGold -= amount
		character.Gold += amount
	}
	gold, cargo := character.Gold, proposed.CargoGold
	_, err := w.submitCriticalWrite(criticalWrite{accounts: []*model.Account{proposed}}, []*Player{p},
		func(criticalPersistenceResult) error {
			// Install only the planned balances. The store copy uses persistence
			// entry positions and must never replace live character identity/state.
			p.Char.Gold, p.Account.CargoGold = gold, cargo
			publishCargoGoldTransfer(p, deposit, amount)
			return nil
		})
	return err
}

func publishCargoGoldTransfer(p *Player, deposit bool, amount uint32) {
	if p.Session.IsClosed() {
		return
	}
	op := uint16(wire.OpWithdraw)
	if deposit {
		op = wire.OpDeposit
	}
	p.Session.Send(wire.CargoGoldTransfer(op, wire.SceneField, amount))
	p.Session.Send(wire.UpdateCargoGold(wire.SceneField, p.Account.CargoGold))
	p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
}
