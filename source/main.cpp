//
// Copyright (C) YuqiaoZhang(HanetakaChou)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <vector>
#include <iostream>
#include <mediapipe/tasks/c/vision/hand_landmarker/hand_landmarker.h>
#include <mediapipe/tasks/c/vision/face_landmarker/face_landmarker.h>
#include <mediapipe/tasks/c/vision/pose_landmarker/pose_landmarker.h>
// #define CV_IGNORE_DEBUG_BUILD_GUARD 1
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include "internal_highgui.h"

// https://ai.google.dev/edge/mediapipe/solutions/vision/hand_landmarker/index#models
// https://storage.googleapis.com/mediapipe-models/hand_landmarker/hand_landmarker/float16/latest/hand_landmarker.task
extern uint8_t const *const hand_landmarker_task_base;
extern size_t const hand_landmarker_task_size;

// https://ai.google.dev/edge/mediapipe/solutions/vision/face_landmarker#models
// https://storage.googleapis.com/mediapipe-models/face_landmarker/face_landmarker/float16/1/face_landmarker.task
extern uint8_t const *const face_landmarker_task_base;
extern size_t const face_landmarker_task_size;

// https://ai.google.dev/edge/mediapipe/solutions/vision/pose_landmarker#models
// https://storage.googleapis.com/mediapipe-models/pose_landmarker/pose_landmarker_lite/float16/latest/pose_landmarker_lite.task
// https://storage.googleapis.com/mediapipe-models/pose_landmarker/pose_landmarker_full/float16/latest/pose_landmarker_full.task
// https://storage.googleapis.com/mediapipe-models/pose_landmarker/pose_landmarker_heavy/float16/latest/pose_landmarker_heavy.task
extern uint8_t const *const pose_landmarker_task_base;
extern size_t const pose_landmarker_task_size;

#define ENABLE_DEBUG_CONSOLE 1
#define ENABLE_HAND_LANDMARKER 1
#define ENABLE_FACE_LANDMARKER 1
#define ENABLE_POSE_LANDMARKER 1

// https://ai.google.dev/edge/mediapipe/solutions/vision/hand_landmarker/index#models
enum
{
    INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_WRIST = 0,
    INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_THUMB_CMC = 1,
    INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_THUMB_MCP = 2,
    INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_THUMB_IP = 3,
    INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_THUMB_TIP = 4,
    INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_INDEX_FINGER_MCP = 5,
    INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_INDEX_FINGER_PIP = 6,
    INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_INDEX_FINGER_DIP = 7,
    INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_INDEX_FINGER_TIP = 8,
    INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_MIDDLE_FINGER_MCP = 9,
    INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_MIDDLE_FINGER_PIP = 10,
    INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_MIDDLE_FINGER_DIP = 11,
    INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_MIDDLE_FINGER_TIP = 12,
    INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_RING_FINGER_MCP = 13,
    INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_RING_FINGER_PIP = 14,
    INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_RING_FINGER_DIP = 15,
    INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_RING_FINGER_TIP = 16,
    INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_PINKY_MCP = 17,
    INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_PINKY_PIP = 18,
    INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_PINKY_DIP = 19,
    INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_PINKY_TIP = 20,
    INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_COUNT = 21
};

// https://ai.google.dev/edge/mediapipe/solutions/vision/pose_landmarker#models
enum
{
    INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_NOSE = 0,
    INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_LEFT_EYE_INNER = 1,
    INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_LEFT_EYE = 2,
    INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_LEFT_EYE_OUTER = 3,
    INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_RIGHT_EYE_INNER = 4,
    INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_RIGHT_EYE = 5,
    INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_RIGHT_EYE_OUTER = 6,
    INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_LEFT_EAR = 7,
    INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_RIGHT_EAR = 8,
    INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_MOUTH_LEFT = 9,
    INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_MOUTH_RIGHT = 10,
    INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_LEFT_SHOULDER = 11,
    INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_RIGHT_SHOULDER = 12,
    INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_LEFT_ELBOW = 13,
    INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_RIGHT_ELBOW = 14,
    INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_LEFT_WRIST = 15,
    INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_RIGHT_WRIST = 16,
    INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_LEFT_PINKY = 17,
    INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_RIGHT_PINKY = 18,
    INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_LEFT_INDEX = 19,
    INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_RIGHT_INDEX = 20,
    INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_LEFT_THUMB = 21,
    INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_RIGHT_THUMB = 22,
    INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_LEFT_HIP = 23,
    INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_RIGHT_HIP = 24,
    INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_LEFT_KNEE = 25,
    INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_RIGHT_KNEE = 26,
    INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_LEFT_ANKLE = 27,
    INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_RIGHT_ANKLE = 28,
    INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_LEFT_HEEL = 29,
    INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_RIGHT_HEEL = 30,
    INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_LEFT_FOOT = 31,
    INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_RIGHT_FOOT = 32,
    INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_COUNT = 33
};

int main(int argc, char *argv[])
{
    {
        constexpr uint32_t const hand_count = 2U;
        constexpr uint32_t const face_count = 1U;
        constexpr uint32_t const pose_count = 1U;
        constexpr char const debug_renderer_window_name[] = {"Press Any Key To Exit"};

        cv::VideoCapture video_capture;
        bool video_capture_open;
        bool is_camera_video_capture;
        if (argc >= 2)
        {
            video_capture_open = video_capture.open(argv[1], cv::CAP_ANY);
            is_camera_video_capture = false;
        }
        else
        {
            video_capture_open = video_capture.open(0, cv::CAP_ANY);
            is_camera_video_capture = true;
        }

        if (!(video_capture_open && video_capture.isOpened()))
        {
            std::cout << "fail to open video capture " << std::endl;
            return -1;
        }
        else
        {
            cv::String backend_name = video_capture.getBackendName();
            std::cout << "video capture backend name: " << backend_name << std::endl;
        }

        if (is_camera_video_capture)
        {
            // Too high resolution may reduce FPS
            video_capture.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
            video_capture.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
            video_capture.set(cv::CAP_PROP_FPS, 30);
        }

        std::cout << "!!!!!!!" << std::endl;
        std::cout << "ATTENTION: you may set the environment variable TFLITE_FORCE_GPU=1 to force OpenCL inference" << std::endl;
        std::cout << "!!!!!!!" << std::endl;

        void *hand_landmarker = NULL;
#if defined(ENABLE_HAND_LANDMARKER) && ENABLE_HAND_LANDMARKER
        {
            HandLandmarkerOptions options;
            options.base_options.model_asset_buffer = reinterpret_cast<char const *>(hand_landmarker_task_base);
            options.base_options.model_asset_buffer_count = static_cast<unsigned int>(hand_landmarker_task_size);
            options.base_options.model_asset_path = NULL;
            options.running_mode = VIDEO;
            options.num_hands = hand_count;
            options.result_callback = NULL;

            char *error_msg_hand_landmarker_create = NULL;
            hand_landmarker = hand_landmarker_create(&options, &error_msg_hand_landmarker_create);
            assert(NULL == error_msg_hand_landmarker_create);
        }
        assert(NULL != hand_landmarker);
#endif

        void *face_landmarker = NULL;
#if defined(ENABLE_FACE_LANDMARKER) && ENABLE_FACE_LANDMARKER
        {
            FaceLandmarkerOptions options;
            options.base_options.model_asset_buffer = reinterpret_cast<char const *>(face_landmarker_task_base);
            options.base_options.model_asset_buffer_count = static_cast<unsigned int>(face_landmarker_task_size);
            options.base_options.model_asset_path = NULL;
            options.running_mode = VIDEO;
            options.num_faces = face_count;
            options.output_face_blendshapes = true;
            options.output_facial_transformation_matrixes = true;
            options.result_callback = NULL;

            char *error_msg_face_landmarker_create = NULL;
            face_landmarker = face_landmarker_create(&options, &error_msg_face_landmarker_create);
            assert(NULL == error_msg_face_landmarker_create);
        }
        assert(NULL != face_landmarker);
#endif

        void *pose_landmarker = NULL;
#if defined(ENABLE_POSE_LANDMARKER) && ENABLE_POSE_LANDMARKER
        {
            PoseLandmarkerOptions options;
            options.base_options.model_asset_buffer = reinterpret_cast<char const *>(pose_landmarker_task_base);
            options.base_options.model_asset_buffer_count = static_cast<unsigned int>(pose_landmarker_task_size);
            options.base_options.model_asset_path = NULL;
            options.running_mode = VIDEO;
            options.num_poses = pose_count;
            options.output_segmentation_masks = false;
            options.result_callback = NULL;

            char *error_msg_pose_landmarker_create = NULL;
            pose_landmarker = pose_landmarker_create(&options, &error_msg_pose_landmarker_create);
            assert(NULL == error_msg_pose_landmarker_create);
        }
#endif

        void *named_window = internal_brx_named_window(debug_renderer_window_name);
        assert(NULL != named_window);

        double const tick_frequency = cv::getTickFrequency();
#if defined(ENABLE_DEBUG_CONSOLE) && ENABLE_DEBUG_CONSOLE
        int64 tick_count_previous = cv::getTickCount();
#endif

        // use the last successful result
        cv::Mat video_frame;

        struct internal_tick_context_t
        {
            cv::VideoCapture &m_video_capture;
            bool const m_is_camera_video_capture;
            void *const m_hand_landmarker;
            void *const m_face_landmarker;
            void *const m_pose_landmarker;
            void *const m_named_window;
            double const m_tick_frequency;
#if defined(ENABLE_DEBUG_CONSOLE) && ENABLE_DEBUG_CONSOLE
            int64 &m_tick_count_previous;
#endif
            cv::Mat &m_video_frame;
        } const internal_tick_context = {
            video_capture,
            is_camera_video_capture,
            hand_landmarker,
            face_landmarker,
            pose_landmarker,
            named_window,
            tick_frequency,
#if defined(ENABLE_DEBUG_CONSOLE) && ENABLE_DEBUG_CONSOLE
            tick_count_previous,
#endif
            video_frame};

        internal_brx_wait_key(
            [](void *tick_context, bool key_pressed) -> bool
            {
                internal_tick_context_t const *const internal_tick_context = static_cast<internal_tick_context_t *>(tick_context);
                cv::VideoCapture &video_capture = internal_tick_context->m_video_capture;
                bool const is_camera_video_capture = internal_tick_context->m_is_camera_video_capture;
                void *const hand_landmarker = internal_tick_context->m_hand_landmarker;
                void *const face_landmarker = internal_tick_context->m_face_landmarker;
                void *const pose_landmarker = internal_tick_context->m_pose_landmarker;
                void *const named_window = internal_tick_context->m_named_window;
                double const tick_frequency = internal_tick_context->m_tick_frequency;
#if defined(ENABLE_DEBUG_CONSOLE) && ENABLE_DEBUG_CONSOLE
                int64 &tick_count_previous = internal_tick_context->m_tick_count_previous;
#endif
                cv::Mat &video_frame = internal_tick_context->m_video_frame;

                if (video_capture.read(video_frame))
                {
                    assert(!video_frame.empty());
                }
                else
                {
                    if (!is_camera_video_capture)
                    {
                        // Loop Playback
                        video_capture.set(cv::CAP_PROP_POS_FRAMES, 0);
                    }
                    else
                    {
                        assert(false);
                    }
                }

                int64 const tick_count_current = cv::getTickCount();

#if defined(ENABLE_DEBUG_CONSOLE) && ENABLE_DEBUG_CONSOLE
                double const fps = tick_frequency / static_cast<double>(tick_count_current - tick_count_previous);
                tick_count_previous = tick_count_current;
                std::cout << "FPS: " << fps << std::endl;
#endif

                size_t const timestamp_ms = static_cast<size_t>((static_cast<double>(tick_count_current) / static_cast<double>(tick_frequency)) * 1000.0);

                if (!video_frame.empty())
                {
                    cv::Mat input_image;
                    {
                        cv::Mat input_rgb_image;

                        cv::cvtColor(video_frame, input_rgb_image, cv::COLOR_BGR2RGB);

                        if ((0U != (static_cast<uint32_t>(input_rgb_image.cols) & (16U - 1U))) || (0U != (static_cast<uint32_t>(input_rgb_image.rows) & (16U - 1U))))
                        {
                            int const width = static_cast<int>(static_cast<uint32_t>(input_rgb_image.cols) & (~(16U - 1U)));
                            int const height = static_cast<int>(static_cast<uint32_t>(input_rgb_image.rows) & (~(16U - 1U)));
                            assert(width <= input_rgb_image.cols);
                            assert(height <= input_rgb_image.rows);

                            if (0 == width || 0 == height)
                            {
                                assert(false);
                                return false;
                            }

                            cv::resize(input_rgb_image, input_image, cv::Size(static_cast<int>(width), static_cast<int>(height)), 0.0, 0.0, cv::INTER_AREA);
                        }
                        else
                        {
                            input_image = std::move(input_rgb_image);
                        }
                    }

                    MpImage mediapipe_input_image;
                    {
                        // mediapipe/examples/desktopdemo_run_graph_main.cc
                        // mediapipe/framework/formats/image_frame_opencv.h
                        // mediapipe/framework/formats/image_frame_opencv.cc

                        constexpr ImageFormat const k_format = SRGB;
                        constexpr int const k_number_of_channels_for_format = 3;
                        constexpr int const k_channel_size_for_format = sizeof(uint8_t);
                        constexpr int const k_mat_type_for_format = CV_8U;

                        constexpr uint32_t const k_default_alignment_boundary = 16U;

                        mediapipe_input_image.type = MpImage::IMAGE_FRAME;
                        mediapipe_input_image.image_frame.format = k_format;
                        mediapipe_input_image.image_frame.width = input_image.cols;
                        mediapipe_input_image.image_frame.height = input_image.rows;

                        int const type = CV_MAKETYPE(k_mat_type_for_format, k_number_of_channels_for_format);
                        int const width_step = (((mediapipe_input_image.image_frame.width * k_number_of_channels_for_format * k_channel_size_for_format) - 1) | (k_default_alignment_boundary - 1)) + 1;
                        assert(type == input_image.type());
                        assert(width_step == input_image.step[0]);
                        mediapipe_input_image.image_frame.image_buffer = static_cast<uint8_t *>(input_image.data);
                        assert(0U == (reinterpret_cast<uintptr_t>(mediapipe_input_image.image_frame.image_buffer) & (k_default_alignment_boundary - 1)));
                        assert(input_image.isContinuous());
                    }

                    HandLandmarkerResult hand_landmarker_result = {};
                    int status_hand_landmarker_detect_for_video = -1;
#if defined(ENABLE_HAND_LANDMARKER) && ENABLE_HAND_LANDMARKER
                    {
                        assert(NULL != hand_landmarker);
                        status_hand_landmarker_detect_for_video = hand_landmarker_detect_for_video(hand_landmarker, &mediapipe_input_image, timestamp_ms, &hand_landmarker_result, NULL);
                    }
#else
                    {
                        assert(NULL == hand_landmarker);
                    }
#endif

                    FaceLandmarkerResult face_landmarker_result = {};
                    int status_face_landmarker_detect_for_video = -1;
#if defined(ENABLE_FACE_LANDMARKER) && ENABLE_FACE_LANDMARKER
                    {
                        assert(NULL != face_landmarker);
                        status_face_landmarker_detect_for_video = face_landmarker_detect_for_video(face_landmarker, &mediapipe_input_image, timestamp_ms, &face_landmarker_result, NULL);
                    }
#else
                    {
                        assert(NULL == face_landmarker);
                    }
#endif

                    PoseLandmarkerResult pose_landmarker_result = {};
                    int status_pose_landmarker_detect_for_video = -1;
#if defined(ENABLE_POSE_LANDMARKER) && ENABLE_POSE_LANDMARKER
                    {
                        assert(NULL != pose_landmarker);
                        status_pose_landmarker_detect_for_video = pose_landmarker_detect_for_video(pose_landmarker, &mediapipe_input_image, timestamp_ms, &pose_landmarker_result, NULL);
                    }
#else
                    {
                        assert(NULL == pose_landmarker);
                    }
#endif

                    {
                        cv::Mat debug_renderer_output_image;

                        {
                            // we do NOT need the input image any more
                            debug_renderer_output_image = std::move(input_image);
                        }

#if defined(ENABLE_HAND_LANDMARKER) && ENABLE_HAND_LANDMARKER
                        if (0 == status_hand_landmarker_detect_for_video)
                        {
                            {
                                cv::Scalar const debug_renderer_hand_color(0, 0, 255);

                                for (uint32_t hand_index = 0U; hand_index < hand_count && hand_index < hand_landmarker_result.hand_landmarks_count; ++hand_index)
                                {
                                    NormalizedLandmarks const &hand_landmark = hand_landmarker_result.hand_landmarks[hand_index];

                                    assert(INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_COUNT == hand_landmark.landmarks_count);

                                    constexpr uint32_t const internal_media_pipe_bones[][2] = {
                                        //
                                        {INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_WRIST, INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_THUMB_CMC},
                                        {INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_THUMB_CMC, INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_THUMB_MCP},
                                        {INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_THUMB_MCP, INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_THUMB_IP},
                                        {INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_THUMB_IP, INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_THUMB_TIP},
                                        //
                                        {INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_WRIST, INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_INDEX_FINGER_MCP},
                                        {INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_INDEX_FINGER_MCP, INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_INDEX_FINGER_PIP},
                                        {INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_INDEX_FINGER_PIP, INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_INDEX_FINGER_DIP},
                                        {INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_INDEX_FINGER_DIP, INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_INDEX_FINGER_TIP},
                                        //
                                        {INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_WRIST, INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_MIDDLE_FINGER_MCP},
                                        {INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_MIDDLE_FINGER_MCP, INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_MIDDLE_FINGER_PIP},
                                        {INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_MIDDLE_FINGER_PIP, INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_MIDDLE_FINGER_DIP},
                                        {INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_MIDDLE_FINGER_DIP, INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_MIDDLE_FINGER_TIP},
                                        //
                                        {INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_WRIST, INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_RING_FINGER_MCP},
                                        {INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_RING_FINGER_MCP, INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_RING_FINGER_PIP},
                                        {INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_RING_FINGER_PIP, INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_RING_FINGER_DIP},
                                        {INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_RING_FINGER_DIP, INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_RING_FINGER_TIP},
                                        //
                                        {INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_WRIST, INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_PINKY_MCP},
                                        {INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_PINKY_MCP, INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_PINKY_PIP},
                                        {INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_PINKY_PIP, INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_PINKY_DIP},
                                        {INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_PINKY_DIP, INTERNAL_MEIDA_PIPE_HAND_POSITION_NAME_PINKY_TIP}};

                                    constexpr uint32_t const bone_count = sizeof(internal_media_pipe_bones) / sizeof(internal_media_pipe_bones[0]);

                                    for (uint32_t bone_index = 0U; bone_index < bone_count; ++bone_index)
                                    {
                                        uint32_t const parent_joint_landmark_index = internal_media_pipe_bones[bone_index][0];
                                        uint32_t const child_joint_landmark_index = internal_media_pipe_bones[bone_index][1];

                                        if ((parent_joint_landmark_index < hand_landmark.landmarks_count) && (child_joint_landmark_index < hand_landmark.landmarks_count))
                                        {
                                            NormalizedLandmark const &parent_normalized_landmark = hand_landmark.landmarks[parent_joint_landmark_index];

                                            NormalizedLandmark const &child_normalized_landmark = hand_landmark.landmarks[child_joint_landmark_index];

                                            if (((!parent_normalized_landmark.has_visibility) || (parent_normalized_landmark.visibility > 0.5F)) && ((!parent_normalized_landmark.has_presence) || (parent_normalized_landmark.presence > 0.5F)) && ((!child_normalized_landmark.has_visibility) || (child_normalized_landmark.visibility > 0.5F)) && ((!child_normalized_landmark.has_presence) || (child_normalized_landmark.presence > 0.5F)))
                                            {
                                                cv::Point const parent_point(static_cast<int>(parent_normalized_landmark.x * debug_renderer_output_image.cols), static_cast<int>(parent_normalized_landmark.y * debug_renderer_output_image.rows));

                                                cv::Point const child_point(static_cast<int>(child_normalized_landmark.x * debug_renderer_output_image.cols), static_cast<int>(child_normalized_landmark.y * debug_renderer_output_image.rows));

                                                cv::line(debug_renderer_output_image, parent_point, child_point, debug_renderer_hand_color);
                                            }
                                        }
                                        else
                                        {
                                            assert(false);
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            assert(false);
                        }
#endif

#if defined(ENABLE_FACE_LANDMARKER) && ENABLE_FACE_LANDMARKER
                        if (0 == status_face_landmarker_detect_for_video)
                        {
                            {
                                for (uint32_t face_index = 0U; face_index < face_count && face_index < face_landmarker_result.face_landmarks_count; ++face_index)
                                {
                                    NormalizedLandmarks const &face_landmark = face_landmarker_result.face_landmarks[face_index];

                                    for (uint32_t landmarks_index = 0U; landmarks_index < face_landmark.landmarks_count; ++landmarks_index)
                                    {
                                        NormalizedLandmark const &normalized_landmark = face_landmark.landmarks[landmarks_index];

                                        if (((!normalized_landmark.has_visibility) || (normalized_landmark.visibility > 0.5F)) && ((!normalized_landmark.has_presence) || (normalized_landmark.presence > 0.5F)))
                                        {
                                            cv::Point point(static_cast<int>(normalized_landmark.x * debug_renderer_output_image.cols), static_cast<int>(normalized_landmark.y * debug_renderer_output_image.rows));
                                            cv::circle(debug_renderer_output_image, point, 1, cv::Scalar(0, 255, 0));
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            assert(false);
                        }
#endif

#if defined(ENABLE_POSE_LANDMARKER) && ENABLE_POSE_LANDMARKER
                        if (0 == status_pose_landmarker_detect_for_video)
                        {
                            {
                                cv::Scalar const debug_renderer_pose_color(255, 0, 0);

                                for (uint32_t pose_index = 0U; pose_index < pose_count && pose_index < pose_landmarker_result.pose_landmarks_count; ++pose_index)
                                {
                                    NormalizedLandmarks const &pose_landmark = pose_landmarker_result.pose_landmarks[pose_index];

                                    assert(INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_COUNT == pose_landmark.landmarks_count);

                                    constexpr uint32_t const internal_media_pipe_bones[][2] = {
                                        //
                                        {INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_RIGHT_SHOULDER, INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_LEFT_SHOULDER},
                                        //
                                        {INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_RIGHT_SHOULDER, INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_RIGHT_ELBOW},
                                        {INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_RIGHT_ELBOW, INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_RIGHT_WRIST},
                                        {INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_RIGHT_WRIST, INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_RIGHT_THUMB},
                                        {INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_RIGHT_WRIST, INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_RIGHT_INDEX},
                                        {INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_RIGHT_WRIST, INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_RIGHT_PINKY},
                                        //
                                        {INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_LEFT_SHOULDER, INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_LEFT_ELBOW},
                                        {INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_LEFT_ELBOW, INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_LEFT_WRIST},
                                        {INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_LEFT_WRIST, INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_LEFT_THUMB},
                                        {INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_LEFT_WRIST, INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_LEFT_INDEX},
                                        {INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_LEFT_WRIST, INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_LEFT_PINKY},
                                        //
                                        {INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_RIGHT_HIP, INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_LEFT_HIP},
                                        //
                                        {INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_RIGHT_HIP, INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_RIGHT_KNEE},
                                        {INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_RIGHT_KNEE, INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_RIGHT_ANKLE},
                                        {INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_RIGHT_ANKLE, INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_RIGHT_FOOT},
                                        //
                                        {INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_LEFT_HIP, INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_LEFT_KNEE},
                                        {INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_LEFT_KNEE, INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_LEFT_ANKLE},
                                        {INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_LEFT_ANKLE, INTERNAL_MEIDA_PIPE_POSE_POSITION_NAME_LEFT_FOOT}};

                                    constexpr uint32_t const bone_count = sizeof(internal_media_pipe_bones) / sizeof(internal_media_pipe_bones[0]);

                                    for (uint32_t bone_index = 0U; bone_index < bone_count; ++bone_index)
                                    {
                                        uint32_t const parent_joint_landmark_index = internal_media_pipe_bones[bone_index][0];
                                        uint32_t const child_joint_landmark_index = internal_media_pipe_bones[bone_index][1];

                                        if ((parent_joint_landmark_index < pose_landmark.landmarks_count) && (child_joint_landmark_index < pose_landmark.landmarks_count))
                                        {
                                            NormalizedLandmark const &parent_normalized_landmark = pose_landmark.landmarks[parent_joint_landmark_index];

                                            NormalizedLandmark const &child_normalized_landmark = pose_landmark.landmarks[child_joint_landmark_index];

                                            if (((!parent_normalized_landmark.has_visibility) || (parent_normalized_landmark.visibility > 0.5F)) && ((!parent_normalized_landmark.has_presence) || (parent_normalized_landmark.presence > 0.5F)) && ((!child_normalized_landmark.has_visibility) || (child_normalized_landmark.visibility > 0.5F)) && ((!child_normalized_landmark.has_presence) || (child_normalized_landmark.presence > 0.5F)))
                                            {
                                                cv::Point const parent_point(static_cast<int>(parent_normalized_landmark.x * debug_renderer_output_image.cols), static_cast<int>(parent_normalized_landmark.y * debug_renderer_output_image.rows));

                                                cv::Point const child_point(static_cast<int>(child_normalized_landmark.x * debug_renderer_output_image.cols), static_cast<int>(child_normalized_landmark.y * debug_renderer_output_image.rows));

                                                cv::line(debug_renderer_output_image, parent_point, child_point, debug_renderer_pose_color);
                                            }
                                        }
                                        else
                                        {
                                            assert(false);
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            assert(false);
                        }
#endif

                        {
                            // Left <-> Right
                            // cv::flip(debug_renderer_output_image, debug_renderer_output_image, 1);

                            cv::Mat debug_renderer_raw_output_image;
                            cv::cvtColor(debug_renderer_output_image, debug_renderer_raw_output_image, cv::COLOR_RGB2BGRA);
                            debug_renderer_output_image.release();

                            void *image_buffer;
                            int32_t image_width;
                            int32_t image_height;
                            {
                                // mediapipe/examples/desktopdemo_run_graph_main.cc
                                // mediapipe/framework/formats/image_frame_opencv.h
                                // mediapipe/framework/formats/image_frame_opencv.cc

                                constexpr int const k_number_of_channels_for_format = 4;
                                constexpr int const k_channel_size_for_format = sizeof(uint8_t);
                                constexpr int const k_mat_type_for_format = CV_8U;

                                constexpr uint32_t const k_default_alignment_boundary = 16U;

                                image_width = debug_renderer_raw_output_image.cols;
                                image_height = debug_renderer_raw_output_image.rows;

                                int const type = CV_MAKETYPE(k_mat_type_for_format, k_number_of_channels_for_format);
                                int const width_step = (((image_width * k_number_of_channels_for_format * k_channel_size_for_format) - 1) | (k_default_alignment_boundary - 1)) + 1;
                                assert(type == debug_renderer_raw_output_image.type());
                                assert(width_step == debug_renderer_raw_output_image.step[0]);
                                image_buffer = static_cast<void *>(debug_renderer_raw_output_image.data);
                                assert(0U == (reinterpret_cast<uintptr_t>(image_buffer) & (k_default_alignment_boundary - 1)));
                                assert(debug_renderer_raw_output_image.isContinuous());
                            }

                            assert(NULL != named_window);
                            internal_brx_image_show(named_window, image_buffer, image_width, image_height);
                        }
                    }

                // should we still close the result when fail?
#if defined(ENABLE_HAND_LANDMARKER) && ENABLE_HAND_LANDMARKER
                    {
                        hand_landmarker_close_result(&hand_landmarker_result);
                    }
#endif

#if defined(ENABLE_FACE_LANDMARKER) && ENABLE_FACE_LANDMARKER
                    {
                        face_landmarker_close_result(&face_landmarker_result);
                    }
#endif

#if defined(ENABLE_POSE_LANDMARKER) && ENABLE_POSE_LANDMARKER
                    {
                        pose_landmarker_close_result(&pose_landmarker_result);
                    }
#endif
                }
                else
                {
                    // use the last successful result when fail
                }

                if (!key_pressed)
                {
                    return true;
                }
                else
                {
                    assert(NULL != named_window);
                    internal_brx_destroy_window(named_window);
                    return false;
                }
            },
            const_cast<internal_tick_context_t *>(&internal_tick_context));

#if defined(ENABLE_HAND_LANDMARKER) && ENABLE_HAND_LANDMARKER
        {
            char *error_msg_hand_landmarker_close = NULL;
            hand_landmarker_close(hand_landmarker, &error_msg_hand_landmarker_close);
            assert(NULL == error_msg_hand_landmarker_close);
            hand_landmarker = NULL;
        }
#endif

#if defined(ENABLE_FACE_LANDMARKER) && ENABLE_FACE_LANDMARKER
        {
            char *error_msg_face_landmarker_close = NULL;
            face_landmarker_close(face_landmarker, &error_msg_face_landmarker_close);
            assert(NULL == error_msg_face_landmarker_close);
            face_landmarker = NULL;
        }
#endif

#if defined(ENABLE_POSE_LANDMARKER) && ENABLE_POSE_LANDMARKER
        {
            char *error_msg_pose_landmarker_close = NULL;
            pose_landmarker_close(pose_landmarker, &error_msg_pose_landmarker_close);
            assert(NULL == error_msg_pose_landmarker_close);
            pose_landmarker = NULL;
        }
#endif

        video_capture.release();
    }

    return 0;
}
