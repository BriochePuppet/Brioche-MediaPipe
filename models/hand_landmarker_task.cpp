#include <stddef.h>
#include <stdint.h>

static constexpr uint8_t const hand_landmarker_task[] = {
#include "bin2h/_internal_hand_landmarker_task.inl"
};

extern uint8_t const *const hand_landmarker_task_base = hand_landmarker_task;

extern size_t const hand_landmarker_task_size = sizeof(hand_landmarker_task);
