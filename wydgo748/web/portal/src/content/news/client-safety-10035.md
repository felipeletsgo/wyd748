---
number: 27
slug: client-safety-10035
date: 2026-10-18
category: News
title: Patch 10035: client stability safeguards
summary: Harden packet and text handling, minimap uploads, actor cleanup and skin-mesh loading.
---

This update adds bounded packet and text handling, safer minimap texture copying and cleanup after aborted UI frames.

Nested network notifications are serialized while packet handlers are active. Actor removal clears nested effect references, and skin-mesh loading validates sizes, bone palettes, positions and indices before publishing resources.

These safeguards do not change skill visuals, rewards or encounter rules. Automated checks passed; the causes of the historical Mana Control, item-drop and heap-corruption incidents remain under investigation.

Close the game and open WYDLauncher.exe to receive patch 10035. Actual gameplay acceptance and broader graphics-resource recovery remain pending.
