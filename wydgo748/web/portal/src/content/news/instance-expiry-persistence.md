---
number: 29
slug: instance-expiry-persistence
date: 2026-10-19
category: News
title: Server update: instance expiry persistence
summary: Keep pending kill rewards and instance exits coordinated when an encounter expires.
---

Instance expiry now waits for pending kill settlement before retiring encounter monsters. Group exits remove room entities only after persistence confirms the exit.

This corrects a reproduced conflict that could put the server into maintenance and prevent login. Rewards, encounter deadlines and the fail-closed policy for uncertain saves are unchanged.

The server update includes the current Hell Gate and Rune Track implementation. Launcher patch 10036 separately delivers the pending control fixes; the persistence correction does not change the client protocol.

Automated regressions and isolated PostgreSQL tests passed. Actual gameplay acceptance remains pending.
