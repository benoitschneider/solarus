/*
 * Copyright (C) 2006-2015 Christopho, Solarus - http://www.solarus-games.org
 *
 * Solarus is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Solarus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "solarus/Camera.h"
#include "solarus/Game.h"
#include "solarus/Map.h"
#include "solarus/entities/MapEntity.h"
#include "solarus/entities/MapEntities.h"
#include "solarus/entities/Hero.h"
#include "solarus/entities/Separator.h"
#include "solarus/movements/TargetMovement.h"
#include "solarus/lowlevel/Video.h"
#include "solarus/lowlevel/System.h"
#include "solarus/lua/LuaContext.h"

#include <algorithm>
#include <list>

namespace Solarus {

/**
 * \brief Creates a camera.
 * \param map The map.
 */
Camera::Camera(Map& map):
  position(Video::get_quest_size()),
  map(map),
  fixed_on(map.get_game().get_hero()),
  separator_scrolling_dx(0),
  separator_scrolling_dy(0),
  separator_next_scrolling_date(0),
  separator_scrolling_direction4(0),
  separator_traversed(),
  restoring(false),
  speed(120),
  movement() {
}

/**
 * \brief Returns the width of the visible area shown by the camera.
 * \return The width of the quest screen.
 */
int Camera::get_width() const {
  return position.get_width();
}

/**
 * \brief Returns the height of the visible area shown by the camera.
 * \return The height of the quest screen.
 */
int Camera::get_height() const {
  return position.get_height();
}

/**
 * \brief Updates the camera position.
 *
 * This function is called continuously by the game loop.
 */
void Camera::update() {

  if (fixed_on != nullptr) {
    // If the camera is not moving towards a target, center it on the hero.
    update_fixed_on();
  }
  else if (movement != nullptr) {
    update_moving();
  }
}

/**
 * \brief Updates the position of the camera when the camera is fixed
 * on the hero.
 */
void Camera::update_fixed_on() {

  Debug::check_assertion(fixed_on != nullptr,
      "Illegal call to Camera::update_fixed_on_hero()");

  int x = 0;
  int y = 0;
  if (separator_next_scrolling_date == 0) {
    // Normal case: not traversing a separator.

    // First compute camera coordinates ignoring map limits and separators.
    const Point& hero_center = fixed_on->get_center_point();
    const int hero_x = hero_center.x;
    const int hero_y = hero_center.y;
    x = hero_x - get_width() / 2;
    y = hero_y - get_height() / 2;

    // Then apply constraints of both separators and map limits.
    this->position = apply_separators_and_map_bounds(Rectangle(x, y, get_width(), get_height()));
  }
  else {
    // The player is currently traversing a separator.
    // Update camera coordinates.
    uint32_t now = System::now();
    bool finished = false;
    while (separator_next_scrolling_date != 0
        && now >= separator_next_scrolling_date) {
      separator_scrolling_position.add_xy(
          separator_scrolling_dx, separator_scrolling_dy
      );

      separator_next_scrolling_date += 1;

      if (separator_scrolling_position == separator_target_position) {
        // Finished.
        finished = true;
      }
    }
    x = separator_scrolling_position.get_x();
    y = separator_scrolling_position.get_y();

    if (finished) {
        separator_next_scrolling_date = 0;
        separator_traversed->notify_activated(separator_scrolling_direction4);
        separator_traversed = nullptr;
        separator_scrolling_direction4 = 0;
    }

    // Then only apply map limit constraints.
    // Ignore separators since we are currently crossing one of them.
    this->position = apply_map_bounds(Rectangle(x, y, get_width(), get_height()));
  }
}

/**
 * \brief Updates the position of the camera when the camera is moving
 * towards a point or back to the hero.
 */
void Camera::update_moving() {

  Debug::check_assertion(fixed_on == nullptr,
      "Illegal call to Camera::update_moving()");

  if (movement == nullptr) {
    return;
  }

  movement->update();
  const Point& xy = movement->get_xy();

  if (movement->is_finished()) {
    movement = nullptr;

    if (restoring) {
      restoring = false;
      fixed_on = map.get_game().get_hero();
      map.get_lua_context().map_on_camera_back(map);
    }
    else {
      map.get_lua_context().notify_camera_reached_target(map);
    }
  }

  position.set_xy(xy);
}

/**
 * \brief Returns whether there is a camera movement.
 *
 * It may be a movement towards a point or a scrolling movement due to a
 * separator.
 *
 * \return \c true if the camera is moving.
 */
bool Camera::is_moving() const {
  return fixed_on == nullptr                  // Moving to a point.
      || separator_next_scrolling_date != 0;  // Traversing a separator.
}

/**
 * \brief Sets the speed of the camera movement.
 * \param speed speed of the movement in pixels per second
 */
void Camera::set_speed(int speed) {
  this->speed = speed;
}

/**
 * \brief Makes the camera move towards a destination point.
 *
 * The camera will be centered on this point.
 * If the camera was already moving, the old movement is discarded.
 *
 * If the target point is not in the same separator region, then the camera
 * will stop on separators.
 *
 * \param target_x X coordinate of the target point.
 * \param target_y Y coordinate of the target point.
 */
void Camera::move(int target_x, int target_y) {

  movement = nullptr;
  fixed_on = nullptr;

  // Take care of the limits of the map and of separators.
  target_x = target_x - get_width() / 2;
  target_y = target_y - get_height() / 2;
  const Rectangle& target_camera = apply_separators_and_map_bounds(
      Rectangle(target_x, target_y, get_width(), get_height())
  );

  movement = std::make_shared<TargetMovement>(
      nullptr, target_camera.get_x(), target_camera.get_y(), speed, true
  );
  movement->set_xy(position.get_xy());
}

/**
 * \brief Makes the camera move towards a destination point.
 *
 * The camera will be centered on this point.
 * If there was already a movement, the new one replaces it.
 *
 * \param target target point
 */
void Camera::move(const Point& target) {
  move(target.x, target.y);
}

/**
 * \brief Makes the camera move towards an entity.
 *
 * The camera will be centered on the entity's center point.
 * If there was already a movement, the new one replaces it.
 * Note that the camera will not update its movement if the entity moves.
 *
 * \param entity the target entity
 */
void Camera::move(Entity& entity) {

  move(entity.get_center_point());
}

/**
 * \brief Moves the camera back to the hero.
 *
 * The hero is not supposed to move during this time.
 * Once the movement is finished, the camera starts following the hero again.
 */
void Camera::restore() {

  move(map.get_entities().get_hero());
  restoring = true;
}

/**
 * \brief Starts traversing a separator.
 *
 * The hero must touch the separator when you call this function.
 *
 * \param separator The separator to traverse (cannot be nullptr).
 */
void Camera::traverse_separator(Separator* separator) {

  Debug::check_assertion(separator != nullptr, "Missing parameter separator");

  // Save the current position of the camera.
  separator_scrolling_position = position;

  // Start scrolling.
  separator_traversed = std::static_pointer_cast<Separator>(
      separator->shared_from_this()
  );
  separator_scrolling_dx = 0;
  separator_scrolling_dy = 0;
  separator_target_position = position;
  Hero& hero = map.get_entities().get_hero();
  const Point& hero_center = hero.get_center_point();
  const Point& separator_center = separator->get_center_point();
  if (separator->is_horizontal()) {
    if (hero_center.y < separator_center.y) {
      separator_scrolling_direction4 = 3;
      separator_scrolling_dy = 1;
      separator_target_position.add_y(get_height());
    }
    else {
      separator_scrolling_direction4 = 1;
      separator_scrolling_dy = -1;
      separator_target_position.add_y(-get_height());
    }
  }
  else {
    if (hero_center.x < separator_center.x) {
      separator_scrolling_direction4 = 0;
      separator_scrolling_dx = 1;
      separator_target_position.add_x(get_width());
    }
    else {
      separator_scrolling_direction4 = 2;
      separator_scrolling_dx = -1;
      separator_target_position.add_x(-get_width());
    }
  }

  separator->notify_activating(separator_scrolling_direction4);
  separator_next_scrolling_date = System::now();

  // Move the hero two pixels ahead to avoid to traversed the separator again.
  hero.set_xy(hero.get_x() + 2 * separator_scrolling_dx,
      hero.get_y() + 2 * separator_scrolling_dy);
}

/**
 * \brief Ensures that a rectangle does not cross map limits.
 * \param area The rectangle to check. Is should not be entirely outside the map.
 * It can be bigger than the map: in such a case, the resulting rectangle cannot
 * avoid to cross map limits, and it will be centered.
 * \return A rectangle corresponding to the first one but stopping on map limits.
 */
Rectangle Camera::apply_map_bounds(const Rectangle& area) {

  int x = area.get_x();  // Top-left corner.
  int y = area.get_y();
  const int width = area.get_width();
  const int height = area.get_height();

  const Size& map_size = map.get_size();
  if (map_size.width < width) {
    x = (map_size.width - width) / 2;
  }
  else {
    x = std::min(std::max(x, 0),
        map_size.width - width);
  }

  if (map_size.height < height) {
    y = (map_size.height - height) / 2;
  }
  else {
    y = std::min(std::max(y, 0),
        map_size.height - height);
  }
  return Rectangle(x, y, width, height);
}

/**
 * \brief Ensures that a rectangle does not cross separators.
 * \param area The rectangle to check.
 * \return A rectangle corresponding to the first one but stopping on separators.
 */
Rectangle Camera::apply_separators(const Rectangle& area) {

  int x = area.get_x();  // Top-left corner.
  int y = area.get_y();
  const int width = area.get_width();
  const int height = area.get_height();

  // TODO simplify: treat horizontal separators first and then all vertical ones.
  int adjusted_x = x;  // Updated coordinates after applying separators.
  int adjusted_y = y;
  std::list<const Separator*> applied_separators;
  const std::list<const Separator*>& separators =
      map.get_entities().get_separators();
  for (const Separator* separator: separators) {

    if (separator->is_vertical()) {
      // Vertical separator.
      int separation_x = separator->get_x() + 8;

      if (x < separation_x && separation_x < x + width
          && separator->get_y() < y + height
          && y < separator->get_y() + separator->get_height()) {
        int left = separation_x - x;
        int right = x + width - separation_x;
        if (left > right) {
          adjusted_x = separation_x - width;
        }
        else {
          adjusted_x = separation_x;
        }
        applied_separators.push_back(separator);
      }
    }
    else {
      Debug::check_assertion(separator->is_horizontal(), "Invalid separator shape");

      // Horizontal separator.
      int separation_y = separator->get_y() + 8;
      if (y < separation_y && separation_y < y + height
          && separator->get_x() < x + width
          && x < separator->get_x() + separator->get_width()) {
        int top = separation_y - y;
        int bottom = y + height - separation_y;
        if (top > bottom) {
          adjusted_y = separation_y - height;
        }
        else {
          adjusted_y = separation_y;
        }
        applied_separators.push_back(separator);
      }
    }
  }  // End for each separator.

  bool must_adjust_x = true;
  bool must_adjust_y = true;
  if (adjusted_x != x && adjusted_y != y) {
    // Both directions were modified. Maybe it is a T configuration where
    // a separator deactivates another one.

    must_adjust_x = false;
    must_adjust_y = false;
    for (const Separator* separator: applied_separators) {

      if (separator->is_vertical()) {
        // Vertical separator.
        int separation_x = separator->get_x() + 8;

        if (x < separation_x && separation_x < x + width
            && separator->get_y() < adjusted_y + height
            && adjusted_y < separator->get_y() + separator->get_height()) {
          must_adjust_x = true;
        }
      }
      else {
        // Horizontal separator.
        int separation_y = separator->get_y() + 8;

        if (y < separation_y && separation_y < y + height
            && separator->get_x() < adjusted_x + width
            && adjusted_x < separator->get_x() + separator->get_width()) {
          must_adjust_y = true;
        }
      }
    }  // End for each separator applied.
  }  // End if both directions.

  if (must_adjust_x) {
    x = adjusted_x;
  }
  if (must_adjust_y) {
    y = adjusted_y;
  }

  return Rectangle(x, y, width, height);
}

/**
 * \brief Ensures that a rectangle does not cross separators nor map bounds.
 * \param area The rectangle to check.
 * It is the responsibility of quest makers to put enough space between
 * separators (the space should be at least the quest size).
 * If separators are too close to each other for the rectangle to fit,
 * the camera will cross some of them.
 * If separators are too close to a limit of the map, the
 * the camera will cross them but will never cross map limits.
 * \return A rectangle corresponding to the first one but stopping on
 * separators and map bounds.
 */
Rectangle Camera::apply_separators_and_map_bounds(const Rectangle& area) {
  return apply_map_bounds(apply_separators(area));
}

}

