DROP TABLE IF EXISTS `ai_playerbot_strategy_action`;
CREATE TABLE `ai_playerbot_strategy_action` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `trigger_id` int(11) unsigned NOT NULL,
  `execution_order` int(11) unsigned NOT NULL DEFAULT '0',
  `name` varchar(128) NOT NULL,
  `priority` float unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `trigger_id` (`trigger_id`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `ai_playerbot_strategy_trigger`;
CREATE TABLE `ai_playerbot_strategy_trigger` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `strategy_id` int(11) unsigned NOT NULL,
  `execution_order` int(11) unsigned NOT NULL DEFAULT '0',
  `name` varchar(128) NOT NULL,
  `flags` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `strategy_id` (`strategy_id`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `ai_playerbot_strategy`;
CREATE TABLE `ai_playerbot_strategy` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `type` tinyint(3) unsigned NOT NULL,
  `name` varchar(128) NOT NULL,
  `description` varchar(1024) NOT NULL DEFAULT '',
  `related_strategies` varchar(1024) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8;
