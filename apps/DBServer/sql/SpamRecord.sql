CREATE DATABASE IF NOT EXISTS global DEFAULT CHARSET utf8 COLLATE utf8_general_ci;

USE global;

CREATE TABLE IF NOT EXISTS SpamRecord
(
	`id` int unsigned not null auto_increment, 
	`read` int unsigned not null default 0,
	`state` int unsigned not null default 0,
	`name` VARCHAR(50),
	`pid` int unsigned,
	`channel` VARCHAR(50),
	`ban_time` int unsigned,
	`content` VARCHAR(256),
	`result` int unsigned,
	`ban_type` int unsigned,
	primary key(id)
);