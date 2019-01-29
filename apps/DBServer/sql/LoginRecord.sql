CREATE TABLE IF NOT EXISTS login_record_%s
(
	`pid` INT UNSIGNED,
	`time` INT UNSIGNED,
	`ip` VARCHAR(16) NOT NULL,
	primary key(`pid`, `time`)
)ENGINE=MyISAM;