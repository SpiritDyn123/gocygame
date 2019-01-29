CREATE DATABASE IF NOT EXISTS account DEFAULT CHARSET utf8 COLLATE utf8_general_ci;

USE account;

CREATE TABLE IF NOT EXISTS player_region
(
	`channel` CHAR(16),
	`channel_id` CHAR(64),
	`region` INT UNSIGNED,
	`player_id` INT UNSIGNED,
	`create_tm` INT UNSIGNED,
	`last_login_tm` INT UNSIGNED,
	PRIMARY KEY(`channel`, `channel_id`, `region`)
)ENGINE=InnoDB DEFAULT CHARSET=utf8;

