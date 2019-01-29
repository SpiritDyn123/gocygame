CREATE DATABASE IF NOT EXISTS global DEFAULT CHARSET utf8 COLLATE utf8_general_ci;

USE global;

CREATE TABLE IF NOT EXISTS qa_info
(
	`qaid` INT UNSIGNED,
	`uid` INT UNSIGNED,
	`seq` INT UNSIGNED,
	`ans` VARCHAR(250),
	PRIMARY KEY(`qaid`, `uid`, `seq`)
);


CREATE TABLE IF NOT EXISTS qa_info_user
(
	`uid` INT UNSIGNED PRIMARY KEY,
	`name` VARCHAR(50)
);



