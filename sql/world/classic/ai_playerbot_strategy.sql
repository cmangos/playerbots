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

INSERT INTO `ai_playerbot_strategy` VALUES ('1', '2', 'bg', 'This strategy will make bots queue up for battle grounds remotely and join them when they get an invite.', 'battleground');
INSERT INTO `ai_playerbot_strategy` VALUES ('2', '2', 'battleground', 'This strategy gives bots basic behavior inside battle grounds like checking and moving to objectives and getting ready at the start gates.', 'bg,warsong,arathi,alterac,eye,isle,arena');
INSERT INTO `ai_playerbot_strategy` VALUES ('3', '0', 'alterac', 'This strategy controls the behavior during an alterac valley battleground.', 'battleground,bg');


INSERT INTO `ai_playerbot_strategy_trigger` VALUES ('1', '1', '0', 'random', '1');
INSERT INTO `ai_playerbot_strategy_trigger` VALUES ('2', '1', '1', 'bg invite active', '1');
INSERT INTO `ai_playerbot_strategy_trigger` VALUES ('3', '2', '0', 'bg waiting', '1');
INSERT INTO `ai_playerbot_strategy_trigger` VALUES ('4', '2', '1', 'bg active', '1');
INSERT INTO `ai_playerbot_strategy_trigger` VALUES ('5', '2', '2', 'very often', '1');
INSERT INTO `ai_playerbot_strategy_trigger` VALUES ('6', '2', '3', 'bg active', '1');
INSERT INTO `ai_playerbot_strategy_trigger` VALUES ('7', '2', '4', 'bg ended', '1');
INSERT INTO `ai_playerbot_strategy_trigger` VALUES ('8', '3', '0', 'very often', '2');


INSERT INTO `ai_playerbot_strategy_action` VALUES ('1', '1', '0', 'bg join', '100');
INSERT INTO `ai_playerbot_strategy_action` VALUES ('2', '2', '1', 'bg status check', '100');
INSERT INTO `ai_playerbot_strategy_action` VALUES ('3', '3', '0', 'bg move to start', '1');
INSERT INTO `ai_playerbot_strategy_action` VALUES ('4', '4', '0', 'check mount state', '2');
INSERT INTO `ai_playerbot_strategy_action` VALUES ('5', '4', '1', 'bg move to objective', '1');
INSERT INTO `ai_playerbot_strategy_action` VALUES ('6', '5', '0', 'bg check objective', '10');
INSERT INTO `ai_playerbot_strategy_action` VALUES ('7', '6', '0', 'bg check flag', '20');
INSERT INTO `ai_playerbot_strategy_action` VALUES ('8', '7', '0', 'bg leave', '20');
INSERT INTO `ai_playerbot_strategy_action` VALUES ('9', '8', '0', 'bg banner', '10');
