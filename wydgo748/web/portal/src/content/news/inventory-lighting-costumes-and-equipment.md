---
number: 19
slug: inventory-lighting-costumes-and-equipment
date: 2026-10-15
category: News
title: Patch 10027: inventory lighting, costumes and equipment
summary: A follow-up for dull item icons, invisible ShopCostum06 costumes, monster reentry and weapon poses.
---

Item icons now use the native fixed inventory lights, white ambient and transparency cutoffs instead of map lighting. This addresses the additional dull-icon defect reported after patch 10025, including unrefined Glihtning Stone. Refinement overlays also use the native grade-to-texture lookup.

Eight imported ShopCostum06 costumes receive explicit renderers and 24 missing mesh files. Changing from two weapons to one refreshes the remaining weapon pose.

A new appearance packet replaces a retired remote actor instead of keeping its pending-deletion state. This fixes one monster reentry defect; it does not establish that every reported Carta disappearance has the same cause.

Close the game and open WYDLauncher.exe to receive patch 10027. Automated checks passed; in-game confirmation remains pending. This is a client-only publication: no game-server update or restart, and the pending Ehre and private-instance HUD server fixes are not included.
