/*
 * Copyright (c) 2019, Train IT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

#define gsl_CONFIG_TRANSPARENT_NOT_NULL 1

#include <gsl/gsl-lite.hpp>
#include <irrlicht-engine/named_type.h>
#include <irrlicht-engine/ranged_value.h>
#include <irrlicht-engine/utils.h>
#include <nonstd/observer_ptr.hpp>
#include <irrlicht.h>
#include <concepts>
#include <filesystem>
#include <memory>
#include <optional>
#include <type_traits>
#include <variant>

namespace workshop {

template<std::derived_from<irr::IReferenceCounted> T>
using droppable_res_ptr = std::unique_ptr<T, decltype([](auto* ptr) { ptr->drop(); })>;

using angle = ranged_value<float, -180, 180>;

// forward declarations
class object_handle;
class engine;

/**
 * @brief Irrlicht selector wrapper class
 *
 * That class is providing the connection between engine and the object to enable objects visibility for collision
 * detection engine and allow possible selection with a laser beam.
 *
 * @note: Throws std::runtime_error in case of problems
 */
class selector : type_counters<selector> {
public:
  explicit selector(engine& e, object_handle& object);

private:
  friend object_handle;
  gsl::not_null<droppable_res_ptr<irr::scene::ITriangleSelector>> resource_;  /// Irrlicht resource
};

/**
 * @brief Irrlicht node wrapper
 *
 * Wraps Irrlicht node interface and provides user friendly interface to create in engine (and refer later on) 1 of 4
 * fixed characters.
 *
 * @note: Throws std::runtime_error in case of problems
 */
class object_handle : type_counters<object_handle> {
public:
  enum class type { faerie, ninja, dwarf, yodan };

  /**
   * Adds new character to workshop::engine
   */
  explicit object_handle(engine& e, type t, const std::string& name);

  /**
   * Creates a reference to already existing Irrlicht node
   */
  explicit object_handle(irr::scene::IAnimatedMeshSceneNode& resource) : resource_(&resource) {}

  void position(float x, float y, float z) { resource_->setPosition(irr::core::vector3df(x, y, z)); }
  void rotation(angle x, angle y, angle z) { resource_->setRotation(irr::core::vector3df(x, y, z)); }
  void selector(selector& s) { resource_->setTriangleSelector(s.resource_.get()); }
  void highlight(bool select) { resource_->setMaterialFlag(irr::video::EMF_LIGHTING, !select); }
  [[nodiscard]] std::string name() const { return resource_->getName(); }
  [[nodiscard]] bool operator==(const object_handle& rhs) const = default;

private:
  friend engine;
  friend workshop::selector;
  gsl::not_null<nonstd::observer_ptr<irr::scene::IAnimatedMeshSceneNode>> resource_;  /// Irrlicht resource
};

// Copyable
static_assert(std::copyable<object_handle>);

/**
 * @brief Irrlicht camera object wrapper
 *
 * @c camera is providing the interface to position engine camera
 * according to user needs.
 *
 * @note: Throws std::runtime_error in case of problems
 */
class camera : type_counters<camera> {
public:
  void position(float x, float y, float z) { resource_.setPosition(irr::core::vector3df(x, y, z)); }
  void target(float x, float y, float z) { resource_.setTarget(irr::core::vector3df(x, y, z)); }

private:
  friend engine;
  irr::scene::ICameraSceneNode& resource_;  /// Irrlicht resource
  explicit camera(irr::scene::ISceneManager& smgr, irr::scene::IMeshSceneNode& level);
};

using stencil_buffer = named_type<bool, class stencil_buffer_>;
using vertical_sync = named_type<bool, class vertical_sync_>;

using window_width = named_type<irr::u32, class window_width_>;
using window_height = named_type<irr::u32, class window_height_>;
struct window_params {
  window_width width;
  window_height height;
};

enum class bits_per_pixel : irr::u32 { bpp_16 = 16, bpp_32 = 32 };
struct full_screen_params {
  window_params window;
  bits_per_pixel bpp;
};

/**
 * @brief 3D Engine based on Irrlicht framework
 *
 * @c engine is the main 3D Engine class responsible for creating and configuring Irrlicht framework. It is also used to
 * create all entities used in our workshop like level, camera, objects and their selectors.
 *
 * @note: Throws std::runtime_error in case of problems
 */
class engine : type_counters<engine> {
public:
  struct irr_runtime {
    irr::video::IVideoDriver& driver;   /// Irrlicht driver interface handler
    irr::scene::ISceneManager& smgr;    /// Irrlicht scene interface handler
    irr::gui::IGUIEnvironment& guienv;  /// Irrlicht GUI interface handler
  };

  enum class device_type {
    null,
    software,
    d3d9,
    opengl,

    last = opengl
  };

  /**
   * @brief Event handler class
   *
   * @c event_receiver class is used to detect keypress needed to end workshop application.
   */
  class event_receiver : public irr::IEventReceiver, type_counters<event_receiver> {
    bool quit_ = false;  /// variable used to exit main loop
  public:
    [[nodiscard]] bool OnEvent(const irr::SEvent& event) override;
    [[nodiscard]] bool quit() const { return quit_; }
  };

  /**
   * Constructor
   *
   * @param irrlicht_media_path  Path to media directory of an Irrlicht library
   * @param screen_params        Window or full screen resolution and depth
   * @param stencil              Enables usage of stencil buffer for shadows
   * @param vsync                Enables vertical sync
   * @param type                 Type of the device to use or default
   */
  explicit engine(std::filesystem::path irrlicht_media_path,
                  const std::variant<window_params, full_screen_params>& screen_params,
                  stencil_buffer stencil = stencil_buffer(true), vertical_sync vsync = vertical_sync(true),
                  device_type type = device_type::software);

  [[nodiscard]] const std::filesystem::path& irrlicht_media_path() const { return irrlicht_media_path_; }
  [[nodiscard]] workshop::camera& camera() { return camera_; }
  [[nodiscard]] const std::optional<object_handle>& selected_object() const { return selected_object_; }

  void draw_label(const std::string& label);

  template<std::invocable Func>
  void run(Func f)
  {
    while (device_->run() && !event_receiver_.quit()) {
      if (device_->isWindowActive()) {
        begin_scene();
        auto _ = gsl::finally([&] { end_scene(); });

        // run user's code
        f();
      } else
        device_->yield();
    }
  }

private:
  friend object_handle;
  friend selector;

  const std::filesystem::path irrlicht_media_path_;  /// path to media directory of the Irrlicht library
  event_receiver event_receiver_;                    /// event receiver

  gsl::not_null<droppable_res_ptr<irr::IrrlichtDevice>>
    device_;                                /// Irrlicht device - the most important object of the engine
  irr_runtime runtime_;                     /// Irrlicht runtime
  irr::gui::IGUIFont& font_;                /// Irrlicht font resource to use
  irr::scene::IBillboardSceneNode& laser_;  /// Irrlicht resource used for laser

  workshop::camera camera_;                       /// engine camera
  std::optional<object_handle> selected_object_;  /// selected object found by collision detection algorithm

  [[nodiscard]] irr_runtime& runtime() { return runtime_; }
  void process_collisions();
  void begin_scene();
  void end_scene();
};

}  // namespace workshop
