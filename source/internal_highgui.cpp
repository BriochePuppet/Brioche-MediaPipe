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

#include "internal_highgui.h"
#include <cstring>
#include <cassert>
#include <string>

#if defined(__GNUC__)
// GCC or CLANG

#include <xcb/xcb.h>

#include "../build-linux/resource.h"

constexpr uint8_t const k_depth = 32U;

struct internal_brx_window
{
    xcb_window_t window;
    xcb_gcontext_t graphics_context;
    xcb_pixmap_t backbuffer_pixmap;
    int32_t window_width;
    int32_t window_height;
};

static xcb_connection_t *s_connection = NULL;
static xcb_screen_t *s_screen = NULL;
static xcb_visualid_t s_visual_id = XCB_NONE;
static xcb_colormap_t s_colormap = XCB_NONE;
static xcb_atom_t s_atom_wm_protocols = XCB_NONE;
static xcb_atom_t s_atom_wm_delete_window = XCB_NONE;
static xcb_pixmap_t s_icon_pixmap = XCB_NONE;

static inline void internal_brx_set_window_icon(xcb_connection_t *connection, xcb_window_t window, xcb_pixmap_t icon_pixmap);

static inline void internal_brx_set_window_size(xcb_connection_t *connection, xcb_window_t window, int32_t width, int32_t height);

extern void *internal_brx_named_window(char const *window_name)
{
    if (NULL == s_connection)
    {
        xcb_connection_t *connection = NULL;
        xcb_screen_t *screen = NULL;
        {
            int screen_number;
            connection = xcb_connect(NULL, &screen_number);
            assert(0 == xcb_connection_has_error(connection));

            xcb_setup_t const *setup = xcb_get_setup(connection);

            int i = 0;
            for (xcb_screen_iterator_t screen_iterator = xcb_setup_roots_iterator(setup); screen_iterator.rem > 0; xcb_screen_next(&screen_iterator))
            {
                if (i == screen_number)
                {
                    screen = screen_iterator.data;
                    break;
                }
                ++i;
            }
        }

        xcb_visualid_t visual_id = XCB_NONE;
        {
            for (xcb_depth_iterator_t depth_iterator = xcb_screen_allowed_depths_iterator(screen); depth_iterator.rem > 0; xcb_depth_next(&depth_iterator))
            {
                if (k_depth == depth_iterator.data->depth)
                {
                    for (xcb_visualtype_iterator_t visual_iterator = xcb_depth_visuals_iterator(depth_iterator.data); visual_iterator.rem > 0; xcb_visualtype_next(&visual_iterator))
                    {
                        if ((XCB_VISUAL_CLASS_TRUE_COLOR == visual_iterator.data->_class) && (0XFF0000U == visual_iterator.data->red_mask) && (0XFF00U == visual_iterator.data->green_mask) && (0XFFU == visual_iterator.data->blue_mask))
                        {
                            visual_id = visual_iterator.data->visual_id;
                            break;
                        }
                    }
                    break;
                }
            }
        }

        xcb_colormap_t colormap = XCB_NONE;
        {
            colormap = xcb_generate_id(connection);

            xcb_void_cookie_t cookie_create_colormap = xcb_create_colormap_checked(connection, XCB_COLORMAP_ALLOC_NONE, colormap, screen->root, visual_id);

            xcb_generic_error_t *error_create_colormap = xcb_request_check(connection, cookie_create_colormap);
            assert(NULL == error_create_colormap);
        }

        xcb_atom_t atom_wm_protocols = XCB_NONE;
        xcb_atom_t atom_wm_delete_window = XCB_NONE;
        {
            xcb_intern_atom_cookie_t cookie_wm_protocols = xcb_intern_atom(connection, 0, 12U, "WM_PROTOCOLS");

            xcb_intern_atom_cookie_t cookie_wm_delete_window = xcb_intern_atom(connection, 0, 16U, "WM_DELETE_WINDOW");

            xcb_generic_error_t *error_intern_atom_reply_wm_protocols;
            xcb_intern_atom_reply_t *reply_wm_protocols = xcb_intern_atom_reply(connection, cookie_wm_protocols, &error_intern_atom_reply_wm_protocols);
            assert(NULL == error_intern_atom_reply_wm_protocols);
            atom_wm_protocols = reply_wm_protocols->atom;
            free(reply_wm_protocols);

            xcb_generic_error_t *error_intern_atom_reply_wm_delete_window;
            xcb_intern_atom_reply_t *reply_wm_delete_window = xcb_intern_atom_reply(connection, cookie_wm_delete_window, &error_intern_atom_reply_wm_delete_window);
            assert(NULL == error_intern_atom_reply_wm_delete_window);
            atom_wm_delete_window = reply_wm_delete_window->atom;
            free(reply_wm_delete_window);
        }

        xcb_pixmap_t icon_pixmap = XCB_NONE;
        {
            icon_pixmap = xcb_generate_id(connection);

            xcb_void_cookie_t cookie_create_icon_pixmap = xcb_create_pixmap_checked(connection, k_depth, icon_pixmap, screen->root, ICON_OPENCV_WIDTH, ICON_OPENCV_HEIGHT);

            xcb_generic_error_t *error_create_icon_pixmap = xcb_request_check(connection, cookie_create_icon_pixmap);
            assert(NULL == error_create_icon_pixmap);
        }

        {
            xcb_gcontext_t icon_pixmap_graphics_context = XCB_NONE;
            {
                assert(XCB_NONE == icon_pixmap_graphics_context);
                icon_pixmap_graphics_context = xcb_generate_id(connection);
                assert(XCB_NONE != icon_pixmap_graphics_context);

                xcb_void_cookie_t cookie_create_graphics_context = xcb_create_gc_checked(connection, icon_pixmap_graphics_context, icon_pixmap, 0U, NULL);

                xcb_generic_error_t *error_create_graphics_context = xcb_request_check(connection, cookie_create_graphics_context);
                assert(NULL == error_create_graphics_context);
            }

            {
                xcb_put_image(connection, XCB_IMAGE_FORMAT_Z_PIXMAP, icon_pixmap, icon_pixmap_graphics_context, ICON_OPENCV_WIDTH, ICON_OPENCV_HEIGHT, 0, 0, 0, k_depth, sizeof(uint32_t) * ICON_OPENCV_WIDTH * ICON_OPENCV_HEIGHT, reinterpret_cast<uint8_t const *>(&ICON_OPENCV_DATA[0][0]));

                int result_flush = xcb_flush(connection);
                assert(result_flush > 0);
            }

            {
                assert(XCB_NONE != icon_pixmap_graphics_context);
                xcb_void_cookie_t cookie_free_graphics_context = xcb_free_gc_checked(connection, icon_pixmap_graphics_context);
                icon_pixmap_graphics_context = XCB_NONE;

                xcb_generic_error_t *error_free_graphics_context = xcb_request_check(connection, cookie_free_graphics_context);
                assert(NULL == error_free_graphics_context);
            }
        }

        assert(NULL == s_connection);
        assert(NULL != connection);
        s_connection = connection;

        assert(NULL == s_screen);
        assert(NULL != screen);
        s_screen = screen;

        assert(XCB_NONE == s_colormap);
        assert(XCB_NONE != colormap);
        s_colormap = colormap;

        assert(XCB_NONE == s_visual_id);
        assert(XCB_NONE != visual_id);
        s_visual_id = visual_id;

        assert(XCB_NONE == s_atom_wm_protocols);
        assert(XCB_NONE != atom_wm_protocols);
        s_atom_wm_protocols = atom_wm_protocols;

        assert(XCB_NONE == s_atom_wm_delete_window);
        assert(XCB_NONE != atom_wm_delete_window);
        s_atom_wm_delete_window = atom_wm_delete_window;

        assert(XCB_NONE == s_icon_pixmap);
        assert(XCB_NONE != icon_pixmap);
        s_icon_pixmap = icon_pixmap;
    }

    void *unwrapped_window_base = aligned_alloc(alignof(internal_brx_window), sizeof(internal_brx_window));
    assert(NULL != unwrapped_window_base);

    internal_brx_window *unwrapped_window = new (unwrapped_window_base) internal_brx_window{XCB_NONE, XCB_NONE, XCB_NONE, 0, 0};
    assert(NULL != unwrapped_window);

    constexpr int32_t const k_window_width = 256;
    constexpr int32_t const k_window_height = 256;

    {
        assert(XCB_NONE == unwrapped_window->window);
        unwrapped_window->window = xcb_generate_id(s_connection);
        assert(XCB_NONE != unwrapped_window->window);

        // Both "border pixel" and "colormap" are required when the depth is NOT equal to the root window's.
        uint32_t value_mask = XCB_CW_BORDER_PIXEL | XCB_CW_BACKING_STORE | XCB_CW_EVENT_MASK | XCB_CW_COLORMAP;

        uint32_t value_list[4] = {0, XCB_BACKING_STORE_NOT_USEFUL, XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_STRUCTURE_NOTIFY, s_colormap};

        xcb_void_cookie_t cookie_create_window = xcb_create_window_checked(s_connection, k_depth, unwrapped_window->window, s_screen->root, 0, 0, k_window_width, k_window_height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT, s_visual_id, value_mask, value_list);

        xcb_generic_error_t *error_create_window = xcb_request_check(s_connection, cookie_create_window);
        assert(NULL == error_create_window);
    }

    {
        xcb_void_cookie_t cookie_change_property_wm_protocols_delete_window = xcb_change_property_checked(s_connection, XCB_PROP_MODE_REPLACE, unwrapped_window->window, s_atom_wm_protocols, XCB_ATOM_ATOM, 8 * sizeof(uint32_t), sizeof(xcb_atom_t) / sizeof(uint32_t), &s_atom_wm_delete_window);

        xcb_generic_error_t *error_change_property_wm_protocols_delete_window = xcb_request_check(s_connection, cookie_change_property_wm_protocols_delete_window);
        assert(NULL == error_change_property_wm_protocols_delete_window);
    }

    {
        xcb_void_cookie_t cookie_change_property_wm_name = xcb_change_property_checked(s_connection, XCB_PROP_MODE_REPLACE, unwrapped_window->window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8 * sizeof(uint8_t), std::strlen(window_name), window_name);

        xcb_generic_error_t *error_change_property_net_wm_name = xcb_request_check(s_connection, cookie_change_property_wm_name);
        assert(NULL == error_change_property_net_wm_name);
    }

    internal_brx_set_window_icon(s_connection, unwrapped_window->window, s_icon_pixmap);

    internal_brx_set_window_size(s_connection, unwrapped_window->window, k_window_width, k_window_height);

    {
        assert(XCB_NONE == unwrapped_window->graphics_context);
        unwrapped_window->graphics_context = xcb_generate_id(s_connection);
        assert(XCB_NONE != unwrapped_window->graphics_context);

        xcb_void_cookie_t cookie_create_graphics_context = xcb_create_gc_checked(s_connection, unwrapped_window->graphics_context, unwrapped_window->window, 0U, NULL);

        xcb_generic_error_t *error_create_graphics_context = xcb_request_check(s_connection, cookie_create_graphics_context);
        assert(NULL == error_create_graphics_context);
    }

    {
        assert(XCB_NONE == unwrapped_window->backbuffer_pixmap);
        unwrapped_window->backbuffer_pixmap = xcb_generate_id(s_connection);
        assert(XCB_NONE != unwrapped_window->backbuffer_pixmap);

        xcb_void_cookie_t cookie_create_pixmap = xcb_create_pixmap_checked(s_connection, k_depth, unwrapped_window->backbuffer_pixmap, unwrapped_window->window, k_window_width, k_window_height);

        xcb_generic_error_t *error_create_pixmap = xcb_request_check(s_connection, cookie_create_pixmap);
        assert(NULL == error_create_pixmap);
    }

    assert(0 == unwrapped_window->window_width);
    unwrapped_window->window_width = k_window_width;
    assert(0 != unwrapped_window->window_width);

    assert(0 == unwrapped_window->window_height);
    unwrapped_window->window_height = k_window_height;
    assert(0 != unwrapped_window->window_height);

    {
        xcb_void_cookie_t cookie_map_window = xcb_map_window_checked(s_connection, unwrapped_window->window);

        xcb_generic_error_t *error_map_window = xcb_request_check(s_connection, cookie_map_window);
        assert(NULL == error_map_window);
    }

    return unwrapped_window;
}

extern void internal_brx_destroy_window(void *wrapped_window)
{
    internal_brx_window *unwrapped_window = static_cast<internal_brx_window *>(wrapped_window);
    assert(NULL != unwrapped_window);

    {
        assert(XCB_NONE != unwrapped_window->backbuffer_pixmap);
        xcb_void_cookie_t cookie_free_backbuffer_pixmap = xcb_free_pixmap_checked(s_connection, unwrapped_window->backbuffer_pixmap);
        unwrapped_window->backbuffer_pixmap = XCB_NONE;

        xcb_generic_error_t *error_free_backbuffer_pixmap = xcb_request_check(s_connection, cookie_free_backbuffer_pixmap);
        assert(NULL == error_free_backbuffer_pixmap);
    }

    {
        assert(XCB_NONE != unwrapped_window->graphics_context);
        xcb_void_cookie_t cookie_free_graphics_context = xcb_free_gc_checked(s_connection, unwrapped_window->graphics_context);
        unwrapped_window->graphics_context = XCB_NONE;

        xcb_generic_error_t *error_free_graphics_context = xcb_request_check(s_connection, cookie_free_graphics_context);
        assert(NULL == error_free_graphics_context);
    }

    {
        assert(XCB_NONE != unwrapped_window->window);
        xcb_void_cookie_t cookie_destroy_window = xcb_destroy_window_checked(s_connection, unwrapped_window->window);
        unwrapped_window->window = XCB_NONE;

        xcb_generic_error_t *error_destroy_window = xcb_request_check(s_connection, cookie_destroy_window);
        assert(NULL == error_destroy_window);
    }

    unwrapped_window->~internal_brx_window();
    free(unwrapped_window);
}

extern void internal_brx_image_show(void *wrapped_window, void const *image_buffer, int image_width, int image_height)
{
    internal_brx_window *unwrapped_window = static_cast<internal_brx_window *>(wrapped_window);
    assert(NULL != unwrapped_window);

    assert(XCB_NONE != unwrapped_window->window);
    assert(XCB_NONE != unwrapped_window->graphics_context);

    if ((image_width != unwrapped_window->window_width) || (image_height != unwrapped_window->window_height))
    {
        internal_brx_set_window_size(s_connection, unwrapped_window->window, image_width, image_height);

        {
            assert(XCB_NONE != unwrapped_window->backbuffer_pixmap);
            xcb_void_cookie_t cookie_free_pixmap = xcb_free_pixmap_checked(s_connection, unwrapped_window->backbuffer_pixmap);
            unwrapped_window->backbuffer_pixmap = XCB_NONE;

            xcb_generic_error_t *error_free_pixmap = xcb_request_check(s_connection, cookie_free_pixmap);
            assert(NULL == error_free_pixmap);
        }

        {
            assert(XCB_NONE == unwrapped_window->backbuffer_pixmap);
            unwrapped_window->backbuffer_pixmap = xcb_generate_id(s_connection);
            assert(XCB_NONE != unwrapped_window->backbuffer_pixmap);

            xcb_void_cookie_t cookie_create_pixmap = xcb_create_pixmap_checked(s_connection, k_depth, unwrapped_window->backbuffer_pixmap, unwrapped_window->window, image_width, image_height);

            xcb_generic_error_t *error_create_pixmap = xcb_request_check(s_connection, cookie_create_pixmap);
            assert(NULL == error_create_pixmap);
        }

        unwrapped_window->window_width = image_width;
        unwrapped_window->window_height = image_height;
    }

    // write "texture" into "back buffer"
    xcb_put_image(s_connection, XCB_IMAGE_FORMAT_Z_PIXMAP, unwrapped_window->backbuffer_pixmap, unwrapped_window->graphics_context, image_width, image_height, 0, 0, 0, k_depth, sizeof(uint32_t) * image_width * image_height, static_cast<uint8_t const *>(image_buffer));

    // copy from "back-buffer" into "front buffer"
    // xcb_present_pixmap(s_connection, unwrapped_window->window, unwrapped_window->backbuffer_pixmap, 0, XCB_NONE, XCB_NONE, 0, 0, XCB_NONE, XCB_NONE, XCB_NONE, XCB_PRESENT_OPTION_NONE, 0, 0, 0, 0, NULL);
    xcb_copy_area(s_connection, unwrapped_window->backbuffer_pixmap, unwrapped_window->window, unwrapped_window->graphics_context, 0, 0, 0, 0, image_width, image_height);

    int result_flush = xcb_flush(s_connection);
    assert(result_flush > 0);
}

extern bool internal_brx_wait_key()
{
    bool pressed_key = false;

    xcb_generic_event_t *event;
    while ((event = xcb_poll_for_event(s_connection)) != NULL)
    {
        // The most significant bit(uint8_t(0X80)) in this code is set if the event was generated from a SendEvent request.
        // https://www.x.org/releases/current/doc/xproto/x11protocol.html#event_format
        switch (event->response_type & (~uint8_t(0X80)))
        {
        case XCB_KEY_PRESS:
        {
            assert(XCB_KEY_PRESS == (event->response_type & (~uint8_t(0X80))));

            pressed_key = true;
        }
        break;
        case XCB_CLIENT_MESSAGE:
        {
            assert(XCB_CLIENT_MESSAGE == (event->response_type & (~uint8_t(0X80))));

            xcb_client_message_event_t *const client_message_event = reinterpret_cast<xcb_client_message_event_t *>(event);
            assert(client_message_event->type == s_atom_wm_protocols && client_message_event->data.data32[0] == s_atom_wm_delete_window);

            // WM_CLOSE

            // Do Nothing
        }
        break;
        case XCB_NONE:
        {
            assert(XCB_NONE == (event->response_type & (~uint8_t(0X80))));

            assert(false);

            // xcb_generic_error_t *error = reinterpret_cast<xcb_generic_error_t *>(event);

            // printf("Error Code: %d Major Code: %d", static_cast<int>(error->error_code), static_cast<int>(error->major_code));
        }
        break;
        default:
        {
            // Do Nothing
        }
        }

        free(event);
    }

    return pressed_key;
}

static inline void internal_brx_set_window_icon(xcb_connection_t *connection, xcb_window_t window, xcb_pixmap_t icon_pixmap)
{
    // xcb/xcb_icccm.h
    constexpr uint32_t const XCB_ICCCM_WM_HINT_ICON_PIXMAP = 1 << 2;

    struct wm_hints_t
    {
        int32_t flags;
        uint32_t input;
        int32_t initial_state;
        xcb_pixmap_t icon_pixmap;
        xcb_window_t icon_window;
        int32_t icon_x, icon_y;
        xcb_pixmap_t icon_mask;
        xcb_window_t window_group;
    };

    wm_hints_t ws_hints = {};
    ws_hints.flags = XCB_ICCCM_WM_HINT_ICON_PIXMAP;
    ws_hints.icon_pixmap = icon_pixmap;

    xcb_void_cookie_t cookie_change_property_wm_hints = xcb_change_property_checked(connection, XCB_PROP_MODE_REPLACE, window, XCB_ATOM_WM_HINTS, XCB_ATOM_WM_HINTS, 8 * sizeof(uint32_t), sizeof(wm_hints_t) / sizeof(uint32_t), &ws_hints);

    xcb_generic_error_t *error_change_property_wm_hints = xcb_request_check(connection, cookie_change_property_wm_hints);
    assert(NULL == error_change_property_wm_hints);
}

static inline void internal_brx_set_window_size(xcb_connection_t *connection, xcb_window_t window, int32_t width, int32_t height)
{
    {
        uint32_t value_mask = XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT;

        uint32_t value_list[2] = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

        xcb_void_cookie_t cookie_configure_window = xcb_configure_window_checked(s_connection, window, value_mask, value_list);

        xcb_generic_error_t *error_configure_window = xcb_request_check(s_connection, cookie_configure_window);
        assert(NULL == error_configure_window);
    }

    {
        // xcb/xcb_icccm.h
        constexpr uint32_t const ICCCM_SIZE_HINT_P_MIN_SIZE = 1 << 4;
        constexpr uint32_t const ICCCM_SIZE_HINT_P_MAX_SIZE = 1 << 5;
        struct size_hints_t
        {
            uint32_t flags;
            int32_t x, y;
            int32_t width, height;
            int32_t min_width, min_height;
            int32_t max_width, max_height;
            int32_t width_inc, height_inc;
            int32_t min_aspect_num, min_aspect_den;
            int32_t max_aspect_num, max_aspect_den;
            int32_t base_width, base_height;
            uint32_t win_gravity;
        };

        size_hints_t size_hints = {};
        size_hints.flags = ICCCM_SIZE_HINT_P_MIN_SIZE | ICCCM_SIZE_HINT_P_MAX_SIZE;
        size_hints.min_width = width;
        size_hints.min_height = height;
        size_hints.max_width = width;
        size_hints.max_height = height;

        xcb_void_cookie_t cookie_change_property_size_hints = xcb_change_property_checked(s_connection, XCB_PROP_MODE_REPLACE, window, XCB_ATOM_WM_NORMAL_HINTS, XCB_ATOM_WM_SIZE_HINTS, 8 * sizeof(uint32_t), sizeof(size_hints_t) / sizeof(uint32_t), &size_hints);

        xcb_generic_error_t *error_change_property_size_hints = xcb_request_check(s_connection, cookie_change_property_size_hints);
        assert(NULL == error_change_property_size_hints);
    }
}

#elif defined(_MSC_VER)
// MSVC or CLANG-CL

#define NOMINMAX 1
#define WIN32_LEAN_AND_MEAN 1
#include <sdkddkver.h>
#include <Windows.h>

#include "../build-windows/resource.h"

struct internal_brx_window
{
    HWND window;
    HDC device_context;
    HDC memory_device_context;
    HBITMAP bitmap;
    int32_t window_width;
    int32_t window_height;
};

extern "C" IMAGE_DOS_HEADER __ImageBase;

static ATOM s_window_class = 0U;

static constexpr DWORD const s_dw_style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
static constexpr DWORD const s_dw_ex_style = WS_EX_APPWINDOW;

static LRESULT CALLBACK s_wnd_proc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

extern void *internal_brx_named_window(char const *const window_name)
{
    HINSTANCE hInstance = reinterpret_cast<HINSTANCE>(&__ImageBase);

    if (0U == s_window_class)
    {
        WNDCLASSEXW const window_class_create_info = {
            sizeof(WNDCLASSEX),
            CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_NOCLOSE,
            s_wnd_proc,
            0,
            0,
            hInstance,
            LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_ICON_OPENCV)),
            LoadCursorW(hInstance, IDC_ARROW),
            (HBRUSH)GetStockObject(NULL_BRUSH),
            NULL,
            L"Brioche Motion",
            LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_ICON_OPENCV)),
        };
        s_window_class = RegisterClassExW(&window_class_create_info);
        assert(0 != s_window_class);
    }

    void *unwrapped_window_base = _aligned_malloc(sizeof(internal_brx_window), alignof(internal_brx_window));
    assert(NULL != unwrapped_window_base);

    internal_brx_window *unwrapped_window = new (unwrapped_window_base) internal_brx_window{NULL, NULL, NULL, NULL, 0, 0};
    assert(NULL != unwrapped_window);

    constexpr int32_t const k_window_width = 256;
    constexpr int32_t const k_window_height = 256;

    {
        std::wstring window_name_utf16;
        {
            assert(window_name_utf16.empty());

            std::string src_utf8 = window_name;
            std::wstring &dst_utf16 = window_name_utf16;

            assert(dst_utf16.empty());

            if (!src_utf8.empty())
            {
                dst_utf16.resize(src_utf8.size() + 1U);

                int written = MultiByteToWideChar(CP_UTF8, 0U, src_utf8.c_str(), src_utf8.size(), dst_utf16.data(), dst_utf16.size());
                assert(written > 0);
                assert(written < dst_utf16.size());
                dst_utf16[written] = L'\0';

                dst_utf16.resize(written);
            }
        }

        HWND const desktop_window = GetDesktopWindow();

        RECT rect;
        {
            HMONITOR const monitor = MonitorFromWindow(desktop_window, MONITOR_DEFAULTTONEAREST);

            MONITORINFOEXW monitor_info;
            monitor_info.cbSize = sizeof(MONITORINFOEXW);
            BOOL res_get_monitor_info = GetMonitorInfoW(monitor, &monitor_info);
            assert(FALSE != res_get_monitor_info);

            rect = RECT{(monitor_info.rcWork.left + monitor_info.rcWork.right) / 2 - k_window_width / 2,
                        (monitor_info.rcWork.bottom + monitor_info.rcWork.top) / 2 - k_window_height / 2,
                        (monitor_info.rcWork.left + monitor_info.rcWork.right) / 2 + k_window_width / 2,
                        (monitor_info.rcWork.bottom + monitor_info.rcWork.top) / 2 + k_window_height / 2};

            BOOL const res_adjust_window_rest = AdjustWindowRectEx(&rect, s_dw_style, FALSE, s_dw_ex_style);
            assert(FALSE != res_adjust_window_rest);
        }

        assert(NULL == unwrapped_window->window);
        unwrapped_window->window = CreateWindowExW(s_dw_ex_style, MAKEINTATOM(s_window_class), window_name_utf16.c_str(), s_dw_style, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, desktop_window, NULL, hInstance, NULL);
        assert(NULL != unwrapped_window->window);
    }

    assert(NULL == unwrapped_window->device_context);
    unwrapped_window->device_context = GetDC(unwrapped_window->window);
    assert(NULL != unwrapped_window->device_context);

    assert(NULL == unwrapped_window->memory_device_context);
    unwrapped_window->memory_device_context = CreateCompatibleDC(unwrapped_window->device_context);
    assert(NULL != unwrapped_window->memory_device_context);

    assert(NULL == unwrapped_window->bitmap);
    unwrapped_window->bitmap = CreateCompatibleBitmap(unwrapped_window->device_context, k_window_width, k_window_height);
    assert(NULL != unwrapped_window->bitmap);

    assert(0 == unwrapped_window->window_width);
    unwrapped_window->window_width = k_window_width;
    assert(0 != unwrapped_window->window_width);

    assert(0 == unwrapped_window->window_height);
    unwrapped_window->window_height = k_window_height;
    assert(0 != unwrapped_window->window_height);

    ShowWindow(unwrapped_window->window, SW_SHOWDEFAULT);

    return static_cast<void *>(unwrapped_window);
}

static LRESULT CALLBACK s_wnd_proc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result;
    switch (Msg)
    {
    case WM_ERASEBKGND:
    {
        assert(WM_ERASEBKGND == Msg);
        result = 1;
    }
    break;
    case WM_CLOSE:
    {
        assert(WM_CLOSE == Msg);
        result = 0;
    }
    break;
    default:
    {
        assert((WM_ERASEBKGND != Msg) && (WM_CLOSE != Msg));
        result = DefWindowProcW(hWnd, Msg, wParam, lParam);
    }
    }
    return result;
}

extern void internal_brx_destroy_window(void *const wrapped_window)
{
    internal_brx_window *unwrapped_window = static_cast<internal_brx_window *>(wrapped_window);
    assert(NULL != unwrapped_window);

    assert(NULL != unwrapped_window->bitmap);
    BOOL result_delete_object = DeleteObject(unwrapped_window->bitmap);
    assert(FALSE != result_delete_object);
    unwrapped_window->bitmap = NULL;

    assert(NULL != unwrapped_window->memory_device_context);
    BOOL result_delete_dc = DeleteDC(unwrapped_window->memory_device_context);
    assert(FALSE != result_delete_dc);
    unwrapped_window->memory_device_context = NULL;

    assert(NULL != unwrapped_window->device_context);
    BOOL result_release_dc = ReleaseDC(unwrapped_window->window, unwrapped_window->device_context);
    assert(FALSE != result_release_dc);
    unwrapped_window->device_context = NULL;

    assert(NULL != unwrapped_window->window);
    BOOL res_destroy_window = DestroyWindow(unwrapped_window->window);
    assert(FALSE != res_destroy_window);
    unwrapped_window->window = NULL;

    unwrapped_window->~internal_brx_window();
    _aligned_free(unwrapped_window);
}

extern void internal_brx_image_show(void *const wrapped_window, void const *const image_buffer, int const image_width, int const image_height)
{
    internal_brx_window *unwrapped_window = static_cast<internal_brx_window *>(wrapped_window);
    assert(NULL != unwrapped_window);

    assert(NULL != unwrapped_window->window);
    assert(NULL != unwrapped_window->device_context);
    assert(NULL != unwrapped_window->memory_device_context);

    if ((image_width != unwrapped_window->window_width) || (image_height != unwrapped_window->window_height))
    {
        {
            RECT rect;
            {
                rect = RECT{0, 0, image_width, image_height};

                BOOL const res_adjust_window_rest = AdjustWindowRectEx(&rect, s_dw_style, FALSE, s_dw_ex_style);
                assert(FALSE != res_adjust_window_rest);
            }

            BOOL res_set_window_pos = SetWindowPos(unwrapped_window->window, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
            assert(FALSE != res_set_window_pos);
        }

        assert(NULL != unwrapped_window->bitmap);
        BOOL result_delete_object = DeleteObject(unwrapped_window->bitmap);
        assert(FALSE != result_delete_object);
        unwrapped_window->bitmap = NULL;

        assert(NULL == unwrapped_window->bitmap);
        unwrapped_window->bitmap = CreateCompatibleBitmap(unwrapped_window->device_context, image_width, image_height);
        assert(NULL != unwrapped_window->bitmap);

        unwrapped_window->window_width = image_width;
        unwrapped_window->window_height = image_height;
    }

    {
        // write "texture" into "back buffer"
        BITMAPINFO bmi;
        ZeroMemory(&bmi, sizeof(bmi));
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = image_width;
        bmi.bmiHeader.biHeight = -image_height;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        int result_set_dib_bits = SetDIBits(unwrapped_window->device_context, unwrapped_window->bitmap, 0, image_height, image_buffer, &bmi, DIB_RGB_COLORS);
        assert(result_set_dib_bits > 0);
    }

    {
        HBITMAP old_bitmap = reinterpret_cast<HBITMAP>(SelectObject(unwrapped_window->memory_device_context, unwrapped_window->bitmap));
        assert(NULL != old_bitmap && HGDI_ERROR != old_bitmap);

        // copy from "back-buffer" into "front buffer"
        int result_bit_blt = BitBlt(unwrapped_window->device_context, 0, 0, image_width, image_height, unwrapped_window->memory_device_context, 0, 0, SRCCOPY);
        assert(0 != result_bit_blt);

        HGDIOBJ new_bitmap = reinterpret_cast<HBITMAP>(SelectObject(unwrapped_window->memory_device_context, old_bitmap));
        assert(new_bitmap == unwrapped_window->bitmap);
    }
}

extern bool internal_brx_wait_key()
{
    bool pressed_key = false;

    MSG msg;
    while (PeekMessageW(&msg, NULL, 0U, 0U, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);

        if (WM_KEYDOWN == msg.message)
        {
            pressed_key = true;
        }
    }

    return pressed_key;
}

#else
#error Unknown Compiler
#endif
