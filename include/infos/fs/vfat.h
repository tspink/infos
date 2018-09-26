/* SPDX-License-Identifier: MIT */
#pragma once

#include <infos/fs/block-based-filesystem.h>
#include <infos/fs/pfs-node.h>

namespace infos
{
	namespace fs
	{
		struct vfat_boot_block {
			char bsp[3];
			char mfr[8];
			uint16_t bytes_per_block;
			uint8_t blocks_per_alloc_unit;
			uint16_t reserved_blocks;
			uint8_t nr_fats;
			uint16_t nr_root_entries;
			uint16_t total_blocks;
			uint8_t media_descriptor;
			uint16_t nr_blocks_in_fat;
			uint16_t nr_blocks_per_track;
			uint16_t nr_heads;
			uint32_t nr_hidden_blocks;
			uint32_t total_blocks_big;
			uint16_t phys_drive_nr;
			uint8_t ebr_signature;
			uint32_t volume_serial_nr;
			char label[11];
			char fsid[8];
			char bspr[0x1c0];
			uint16_t signature;
		} __packed;

		static_assert(sizeof(vfat_boot_block) == 512, "VFAT BOOT BLOCK not correct size");

		class VFAT : public BlockBasedFilesystem
		{
		public:
			VFAT(drivers::block::BlockDevice& bdev);
			PFSNode *mount() override;
			const util::String name() const { return "vfat"; }
			
		private:
			struct vfat_boot_block _boot_block;
		};

		class VFATNode : public PFSNode
		{
		public:
			VFATNode(VFAT& fs);

			PFSNode* get_child(const util::String& name) override;
			PFSNode* mkdir(const util::String& name) override;
			
			File* open() override;
			Directory* opendir() override;
		};
	}
}
