#include "../internal/game/entities/HumanCostumeRefinement.h"

#include <algorithm>
#include <cstdio>
#include <iterator>
#include <limits>

namespace {
// Independent explicit membership from the pre-extraction renderer branches.
constexpr int fixedCostumes[] = {
    4150, 4151, 4169, 4170, 4171, 4172, 4173, 4176, 4177, 4178, 4179, 4183,
    4300, 4301, 4302, 4303, 4304, 4305, 4309, 4310, 4311, 4312, 4313, 4314,
    4315, 4316, 4317, 4318, 4320, 4321, 4322, 4323, 4324, 4325, 4326, 4327,
    4328, 4329, 4330, 4331, 4332, 4333, 4334, 4335, 4336, 4337, 4338, 4339,
    4340, 4341, 4342, 4343, 4344, 4345, 4346, 4347, 4348, 4349, 4350, 4351,
    4352, 4353, 4354, 4355, 4356, 4357, 4358, 4359, 4360, 4361, 4362, 4363,
    4364, 4365, 4366, 4367, 4368, 4369, 4370, 4371, 4372, 4373, 4374, 4376,
    4377, 4378, 4379, 4380, 4381, 4382, 4383, 4384, 4385, 4386, 4387, 4388,
    4389, 4390, 4391, 4392, 4393, 4394, 4395, 4396, 4397, 4398, 4399, 4400,
    4401, 4402, 4403, 4404, 4405, 4406, 4407, 4408, 4409, 4410, 4411, 4412,
    4413, 4414, 4415, 4416, 4417, 4418, 4419, 4420,
};
static_assert(std::size(fixedCostumes) == 128);
static_assert(human_costume::UsesFixedBodyRefinement(4150));
static_assert(!human_costume::UsesFixedBodyRefinement(4152));
static_assert(!human_costume::UsesFixedBodyRefinement(4375));
static_assert(human_costume::UsesFixedBodyRefinement(4420));
static_assert(!human_costume::UsesFixedBodyRefinement(4421));
}

int RunHumanCostumeRefinementTests(int& checks)
{
    int failures = 0;
    const auto check = [&](int costume) {
        ++checks;
        const bool expected = std::find(std::begin(fixedCostumes),
            std::end(fixedCostumes), costume) != std::end(fixedCostumes);
        if (human_costume::UsesFixedBodyRefinement(costume) != expected) {
            ++failures;
            std::fprintf(stderr, "FAIL human costume refinement: %d\n", costume);
        }
    };
    // m_sCostume is a signed short: cover every representable runtime input.
    for (int costume = (std::numeric_limits<short>::min)();
         costume <= (std::numeric_limits<short>::max)(); ++costume)
        check(costume);
    // The pure API also rejects inputs outside the runtime storage domain.
    for (int costume : {(std::numeric_limits<int>::min)(), -32769, 32768,
        (std::numeric_limits<int>::max)()})
        check(costume);
    return failures;
}
