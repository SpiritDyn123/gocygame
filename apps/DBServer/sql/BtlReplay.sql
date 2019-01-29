CREATE TABLE IF NOT EXISTS btl_replay_%s
(
	`key` VARCHAR(250) PRIMARY KEY,
	`uid` INT UNSIGNED,
	`refCnt` INT,
	`data` MEDIUMBLOB,
	`time` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
	`ver` VARCHAR(128),
	INDEX `btl_replay_uid_time` (`uid`, `time`)
)ENGINE=MyISAM;
