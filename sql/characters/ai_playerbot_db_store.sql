DROP TABLE IF EXISTS `ai_playerbot_db_store`;

CREATE TABLE `ai_playerbot_db_store` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `guid` bigint(20) NOT NULL,
  `preset` varchar(32) NOT NULL,
  `key` varchar(32) NOT NULL,
  `value` varchar(4000) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `guid` (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;