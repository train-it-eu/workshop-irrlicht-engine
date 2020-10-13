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

#include "utils.h"
#include <irrlicht.h>

namespace workshop {

// forward declarations
class object_handle;
class engine;

#define SELECTOR_INIT_FAIL 0
#define SELECTOR_INIT_SUCCESS 1

/**
 * @brief Irrlicht selector wrapper class
 *
 * That class is providing the connection between engine and the object to enable objects visibility for collision
 * detection engine and allow possible selection with a laser beam.
 */
class selector : type_counters<selector> {
public:
  selector();
  ~selector();

  /**
   * Initializes resource
   *
   * @param engine Irrlicht Engine
   * @param object Object to connect
   *
   * @return Initialization status
   */
  int init(engine* e, object_handle* object);

  /**
   * Releases resource
   */
  void destroy();

private:
  friend object_handle;
  irr::scene::ITriangleSelector* resource_;  /// Irrlicht resource
};

/**
 * @brief Irrlicht node wrapper
 *
 * Wraps Irrlicht node interface and provides user friendly interface to create in engine (and refer later on) 1 of 4
 * fixed characters.
 */
class object_handle : type_counters<object_handle> {
public:
  enum type { type_unknown = -2, type_invalid = -1, type_faerie, type_ninja, type_dwarf, type_yodan, type_num };

  object_handle(type t, const std::string* name);
  bool resource_set(engine* e);
  bool resource_set(irr::scene::IAnimatedMeshSceneNode* resource);
  void position(float x, float y, float z);
  void rotation(float x, float y, float z);
  void selector(selector* s);
  void highlight(bool select);
  void name(std::string* name) const;
  bool operator==(const object_handle& rhs) const;

private:
  friend engine;
  friend workshop::selector;
  type type_;                                     /// cached object type
  const std::string* name_;                       /// used temporarily during construction
  irr::scene::IAnimatedMeshSceneNode* resource_;  /// Irrlicht resource
};

/**
 * @brief Irrlicht camera object wrapper
 *
 * @c camera is providing the interface to position engine camera
 * according to user needs.
 */
class camera : type_counters<camera> {
public:
  camera();
  void position(float x, float y, float z);
  void target(float x, float y, float z);

private:
  friend engine;
  irr::scene::ICameraSceneNode* resource_;  /// Irrlicht resource
  int init(irr::scene::ISceneManager* smgr, irr::scene::IMeshSceneNode* level);
};

/**
 * @brief 3D Engine based on Irrlicht framework
 *
 * @c engine is the main 3D Engine class responsible for creating and configuring Irrlicht framework. It is also used to
 * create all entities used in our workshop like level, camera, objects and their selectors.
 */
class engine : type_counters<engine> {
public:
  struct irr_runtime {
    irr::video::IVideoDriver* driver;   /// Irrlicht driver interface handler
    irr::scene::ISceneManager* smgr;    /// Irrlicht scene interface handler
    irr::gui::IGUIEnvironment* guienv;  /// Irrlicht GUI interface handler
  };

  enum device_type { device_invalid = -1, device_null, device_software, device_d3d9, device_opengl, device_num };

  /**
   * @brief Event handler class
   *
   * @c event_receiver class is used to detect keypress needed to end workshop application.
   */
  class event_receiver : public irr::IEventReceiver, type_counters<event_receiver> {
  public:
    bool quit_;  /// variable used to exit main loop
    event_receiver() : quit_(false) {}
    virtual bool OnEvent(const irr::SEvent& event);
  };

  /**
   * Constructor
   *
   * @param irrlicht_path  Path to main directory of Irrlicht library
   * @param type           Type of the device to use or default if null
   */
  engine(const std::string& irrlicht_path, device_type* type);
  ~engine();

  const std::string& irrlicht_path() const { return irrlicht_path_; }

  /**
   * Creates and returns camera
   *
   * @param c Created object
   *
   * @return Error code
   */
  int create_camera(camera** c);

  /**
   * Destroys camera
   */
  void destroy_camera();

  /**
   * Initializes device resource
   *
   * @param width          Window/Screen width
   * @param height         Window/Screen height
   * @param bpp            Bits per pixel valid only in full screen mode (16 or 32)
   * @param full_screen    Enables full screen mode
   * @param stencil        Enables usage of stencil buffer for shadows
   * @param vsync          Enables vertical sync
   *
   * @return Error code
   */
  int init_device(int width, int height, int bpp, bool full_screen, bool stencil, bool vsync);

  /**
   * Adds custom font to the engine
   *
   * @return Status
   */
  bool font();

  /**
   * Configures laser
   *
   * @return Status
   */
  bool add_laser();

  /**
   * Adds a light so it is not dark out there
   *
   * @return Error code
   */
  int add_light();

  /**
   * Returns selected object
   *
   * @return Selected object
   */
  object_handle* selected_object() const { return selected_object_; }

  /**
   * Draws custom label
   *
   * @param label Label to draw
   */
  void draw_label(const std::string& label);

  /**
   * Creates internal event receiver
   *
   * @return Status
   */
  bool internal_event_receiver_create();

  /**
   * @brief Runs the engine
   *
   * Should be used in such a way:
   * @code
   *   while(engine->run()) {
   *     if(engine->window_active()) {
   *       engine->begin_scene();
   *
   *       // draw everything here
   *
   *       engine->end_scene();
   *     }
   *     else
   *       engine->yield();
   *   }
   * @endcode
   *
   * @return Status
   */
  bool run();
  bool window_active();
  bool begin_scene();
  bool end_scene();
  void yield();

private:
  friend object_handle;
  friend selector;

  const std::string irrlicht_path_;  /// path to Irrlicht engine library
  device_type device_type_;          /// device type
  event_receiver* event_receiver_;   /// event receiver

  irr::IrrlichtDevice* device_;             /// Irrlicht device - the most important object of the engine
  irr_runtime runtime_;                     /// Irrlicht runtime
  irr::gui::IGUIFont* font_;                /// Irrlicht font resource to use
  irr::scene::IBillboardSceneNode* laser_;  /// Irrlicht resource used for laser

  camera* camera_;                  /// engine camera
  object_handle* selected_object_;  /// selected object found by collision detection algorithm

  irr::video::E_DRIVER_TYPE convert(device_type type);
  int add_level(irr::scene::IMeshSceneNode** level);
  irr_runtime* runtime() { return &runtime_; }
  int process_collisions();
};

}  // namespace workshop
