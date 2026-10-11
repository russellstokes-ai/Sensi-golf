extends RefCounted
class_name RecoveredReference

# DEVELOPER-ONLY: authentic original-derived numeric evidence.
# Never drive the Pixtee shot model from this script; use PixteeGameplay
# (the already recovered C++ ClassicHoleSession) for gameplay authority.
const DATA_FILE := "res://reference/ORIGINAL_PHYSICS_RESEARCH.json"
const ROUND_FILE := "res://reference/ORIGINAL_ROUND_REFERENCE.txt"

static func load_verified_research() -> Dictionary:
    var contents := FileAccess.get_file_as_string(DATA_FILE)
    if contents.is_empty():
        push_error("Missing verified Sensible Golf research bank")
        return {}
    var parsed = JSON.parse_string(contents)
    if not (parsed is Dictionary):
        push_error("Malformed original physics research bank")
        return {}
    var data: Dictionary = parsed
    if data.get("schema") != "pixtee-research.original-physics.v1":
        push_error("Unknown Sensible Golf evidence bank version")
        return {}
    if data.get("recovered_club_launch_research", []).size() != 13 or data.get("recovered_terrain_descriptor_research", []).size() != 77:
        push_error("Recovered research bank is incomplete")
        return {}
    return data

static func research_counts() -> Dictionary:
    var data := load_verified_research()
    if data.is_empty():
        return {"verified_reference_loaded": false}
    return {
        "verified_reference_loaded": true,
        "original_game_reference": "Sensible Golf Windows v1.014",
        "source_executable_sha256": data.get("source_executable_sha256", ""),
        "club_rows": data["recovered_club_launch_research"].size(),
        "club_lie_rows": data["recovered_club_lie_profile_selectors"].size(),
        "accuracy_profiles": data["recovered_swing_profile_samples"].size(),
        "terrain_descriptors": data["recovered_terrain_descriptor_research"].size(),
        "rights_cleared_for_distribution": false,
        "simulation_uses_gdscript_tables": false,
    }
