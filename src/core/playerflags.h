#pragma once

#include <cstdint>
#include <string>

typedef uint32_t playerflags_t;

struct PlayerFlags {
	PlayerFlags() : flags(0) {}
	PlayerFlags(playerflags_t pf) : flags(pf) {}

	inline bool check(playerflags_t cf) const
	{
		return (flags & cf) != 0;
	}
	inline void set(playerflags_t nf, playerflags_t mask)
	{
		flags = (flags & ~mask) | nf;
	}
	std::string toHumanReadable() const;
	static bool stringToPlayerFlags(const std::string &input, playerflags_t *out);

	playerflags_t flags;

	// Only valid within the current world
	enum PlayerFlagsEnum : playerflags_t {
		// World access
		PF_BANNED    = 0x00000001,
		PF_EDIT      = 0x00000002,
		PF_EDIT_DRAW = 0x00000004 | PF_EDIT,
		PF_GODMODE   = 0x00000008,
		PF_HELPER    = 0x00000100 | PF_EDIT_DRAW | PF_GODMODE,
		PF_OWNER     = 0x00000800 | PF_HELPER, // not saved for the actual owner
		// Temporary
		PF_TMP_HEAVYKICK = 0x01000000,
		PF_TMP_EDIT      = 0x02000000,
		PF_TMP_EDIT_DRAW = 0x04000000 | PF_TMP_EDIT,
		PF_TMP_GODMODE   = 0x08000000,
		// Masks for priv checks
		PF_MASK_EDIT      = PF_EDIT | PF_TMP_EDIT,
		PF_MASK_EDIT_DRAW = PF_EDIT_DRAW | PF_TMP_EDIT_DRAW,
		PF_MASK_GODMODE   = PF_GODMODE | PF_TMP_GODMODE,
		// For saving to the database
		PF_MASK_SAVE = PF_BANNED | PF_OWNER,
		// For network transmission: broadcast
		PF_MASK_SEND_BROADC = PF_MASK_EDIT_DRAW | PF_MASK_GODMODE,
		// For network transmission: per-player
		PF_MASK_SEND_PLAYER = PF_MASK_SAVE | PF_TMP_EDIT_DRAW | PF_TMP_GODMODE,
	};
};



