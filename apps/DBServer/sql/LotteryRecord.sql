CREATE TABLE IF NOT EXISTS lottery_record_%s
(
	`pid` INT UNSIGNED,
	`type` INT UNSIGNED,
	`time` INT UNSIGNED,
	`data` MEDIUMBLOB,
	primary key(`pid`, `type`, `time`)
)ENGINE=MyISAM;