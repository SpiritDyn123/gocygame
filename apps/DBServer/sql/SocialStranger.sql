CREATE TABLE IF NOT EXISTS social_stranger_%s
(
	`pid` INT UNSIGNED,
	`stranger_pid` INT UNSIGNED,
	`time` BIGINT,
	primary key(`pid`, `stranger_pid`)
)ENGINE=MyISAM;
