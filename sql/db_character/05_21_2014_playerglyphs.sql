ALTER TABLE `playerglyphs` CHANGE COLUMN `glyph1` `glyph1` MEDIUMINT UNSIGNED NULL DEFAULT NULL AFTER `spec`;
ALTER TABLE `playerglyphs` CHANGE COLUMN `glyph2` `glyph2` MEDIUMINT UNSIGNED NULL DEFAULT NULL AFTER `glyph1`;
ALTER TABLE `playerglyphs` CHANGE COLUMN `glyph3` `glyph3` MEDIUMINT UNSIGNED NULL DEFAULT NULL AFTER `glyph2`;
ALTER TABLE `playerglyphs` CHANGE COLUMN `glyph4` `glyph4` MEDIUMINT UNSIGNED NULL DEFAULT NULL AFTER `glyph3`;
ALTER TABLE `playerglyphs` CHANGE COLUMN `glyph5` `glyph5` MEDIUMINT UNSIGNED NULL DEFAULT NULL AFTER `glyph4`;
ALTER TABLE `playerglyphs` CHANGE COLUMN `glyph6` `glyph6` MEDIUMINT UNSIGNED NULL DEFAULT NULL AFTER `glyph5`;
ALTER TABLE `playerglyphs` ADD COLUMN `glyph7` MEDIUMINT(8) UNSIGNED NULL DEFAULT NULL AFTER `glyph6`;
ALTER TABLE `playerglyphs` ADD COLUMN `glyph8` MEDIUMINT(8) UNSIGNED NULL DEFAULT NULL AFTER `glyph7`;
ALTER TABLE `playerglyphs` ADD COLUMN `glyph9` MEDIUMINT(8) UNSIGNED NULL DEFAULT NULL AFTER `glyph8`;