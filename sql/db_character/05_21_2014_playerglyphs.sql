DROP TABLE IF EXISTS `playerglyphs`;
CREATE TABLE `playerglyphs` (
    `guid` INT(10) UNSIGNED NOT NULL,
    `spec` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
    `glyph1` MEDIUMINT(8) UNSIGNED NULL DEFAULT NULL,
    `glyph2` MEDIUMINT(8) UNSIGNED NULL DEFAULT NULL,
    `glyph3` MEDIUMINT(8) UNSIGNED NULL DEFAULT NULL,
    `glyph4` MEDIUMINT(8) UNSIGNED NULL DEFAULT NULL,
    `glyph5` MEDIUMINT(8) UNSIGNED NULL DEFAULT NULL,
    `glyph6` MEDIUMINT(8) UNSIGNED NULL DEFAULT NULL,
    `glyph7` MEDIUMINT(8) UNSIGNED NULL DEFAULT NULL,
    `glyph8` MEDIUMINT(8) UNSIGNED NULL DEFAULT NULL,
    `glyph9` MEDIUMINT(8) UNSIGNED NULL DEFAULT NULL,
  PRIMARY KEY (`guid`,`spec`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 CHECKSUM=1 DELAY_KEY_WRITE=1 ROW_FORMAT=DYNAMIC;