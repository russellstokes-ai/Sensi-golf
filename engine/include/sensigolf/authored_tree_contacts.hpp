#pragma once
#include <cstdint>
#include <vector>
namespace sensigolf {
// Owner-authorized Pixtee geometry, NOT claimed as original tree-detector parity.
// All coordinates/radii/heights are fixed 16.16 world units.
struct AuthoredTreeVolume {
    std::uint32_t id=0;
    std::int32_t x=0,y=0,trunk_radius=0,trunk_height=0;
    std::int32_t canopy_radius=0,canopy_bottom=0,canopy_top=0;
};
struct AuthoredTreePoint {std::int32_t x=0,y=0,z=0;};
enum class AuthoredTreeContactKind {None,Trunk,Canopy};
struct AuthoredTreeContact {
    std::uint32_t tree_id=0;
    AuthoredTreeContactKind kind=AuthoredTreeContactKind::None;
};
void validate_authored_trees(const std::vector<AuthoredTreeVolume>& trees);
AuthoredTreeContact authored_tree_contact(const std::vector<AuthoredTreeVolume>& trees,
    AuthoredTreePoint from,AuthoredTreePoint to);
}
