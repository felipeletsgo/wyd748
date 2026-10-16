#pragma once

namespace entity_reentry {
// CreateMob is authoritative entry, not a refresh of an actor already removed
// from view. Keep live duplicates stable, but never let deferred deletion hide
// a new entry with the same ID. Login alone owns the local player's lifetime.
constexpr bool Replace(bool isSelf, int willDie, unsigned int delayedDelete,
    bool deleted)
{
    return !isSelf && (willDie != -1 || delayedDelete != 0 || deleted);
}
}
