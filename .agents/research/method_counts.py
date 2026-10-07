"""Truthful method-preservation counts for the relocation gates (audit A06).

Every category is derived from the baseline and current signature -> digest
maps. A body counts as identical only when its normalized digest is the
baseline's; pinned extractions and reviewed changes are reported apart, and an
approval label never implies the body was exercised at runtime.
"""


def categories(baseline, actual, pinned=(), reviewed=()):
    pinned, reviewed = set(pinned), set(reviewed)
    both = baseline.keys() & actual.keys()
    changed = {s for s in both if baseline[s] != actual[s]}
    return {
        'baseline': len(baseline),
        'current': len(actual),
        'identical': len(both) - len(changed),
        'reviewed': len({s for s in changed if s in reviewed}),
        'pinned': len({s for s in changed if s not in reviewed and s in pinned}),
        'unexplained': sorted(s for s in changed if s not in reviewed and s not in pinned),
        'added': len(actual.keys() - baseline.keys()),
        'removed': len(baseline.keys() - actual.keys()),
    }


def summary(counts):
    return (f"{counts['baseline']} baseline / {counts['current']} current definitions: "
            f"{counts['identical']} identical normalized bodies, {counts['pinned']} pinned extractions, "
            f"{counts['reviewed']} reviewed changes, {counts['added']} added, {counts['removed']} removed")
