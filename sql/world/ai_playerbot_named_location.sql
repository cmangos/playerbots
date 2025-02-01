DROP TABLE IF EXISTS `ai_playerbot_named_location`;
CREATE TABLE `ai_playerbot_named_location` (
  `name` char(128) NOT NULL,
  `map_id` smallint(5) NOT NULL,
  `position_x` decimal(40,20) NOT NULL,
  `position_y` decimal(40,20) NOT NULL,
  `position_z` decimal(40,20) NOT NULL,
  `orientation` decimal(40,20) NOT NULL,
  `description` varchar(255) NOT NULL,
  PRIMARY KEY (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='PlayerbotAI Named Location';
