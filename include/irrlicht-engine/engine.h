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
#include <irrlicht-engine/utils.h>
#include <nonstd/observer_ptr.hpp>
#include <irrlicht.h>
#include <filesystem>
#include <memory>
#include <optional>
#include <type_traits>

namespace workshop {

template<typename T>
using droppable_res_ptr = std::unique_ptr<T, decltype([](auto* ptr) { ptr->drop(); })>;

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
  void rotation(float x, float y, float z);
  void selector(selector& s) { resource_->setTriangleSelector(s.resource_.get()); }
  void highlight(bool select) { resource_->setMaterialFlag(irr::video::EMF_LIGHTING, !select); }
  std::string name() const { return resource_->getName(); }
  bool operator==(const object_handle& rhs) const = default;

private:
  friend engine;
  friend workshop::selector;
  gsl::not_null<nonstd::observer_ptr<irr::scene::IAnimatedMeshSceneNode>> resource_;  /// Irrlicht resource
};

// Copyable
static_assert(std::is_copy_constructible_v<object_handle>);
static_assert(std::is_copy_assignable_v<object_handle>);

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
    virtual bool OnEvent(const irr::SEvent& event);
    bool quit() const { return quit_; }
  };

  /**
   * Constructor
   *
   * @param irrlicht_media_path  Path to media directory of an Irrlicht library
   * @param width                Window/Screen width
   * @param height               Window/Screen height
   * @param bpp                  Bits per pixel valid only in full screen mode (16 or 32)
   * @param full_screen          Enables full screen mode
   * @param stencil              Enables usage of stencil buffer for shadows
   * @param vsync                Enables vertical sync
   * @param type                 Type of the device to use or default
   */
  explicit engine(std::filesystem::path irrlicht_media_path, irr::u32 width, irr::u32 height, irr::u32 bpp,
                  bool full_screen, bool stencil, bool vsync, device_type type = device_type::software);

  const std::filesystem::path& irrlicht_media_path() const { return irrlicht_media_path_; }
  workshop::camera& camera() { return camera_; }
  const std::optional<object_handle>& selected_object() const { return selected_object_; }

  void draw_label(const std::string& label);

  template<typename Func>
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

  irr_runtime& runtime() { return runtime_; }
  void process_collisions();
  void begin_scene();
  void end_scene();
};

}  // namespace workshop
