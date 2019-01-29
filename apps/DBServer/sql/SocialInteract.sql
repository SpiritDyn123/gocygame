CREATE TABLE IF NOT EXISTS social_interact_%s
(
	`pid` INT UNSIGNED,
	`interact_pid` INT UNSIGNED,
	`type` INT,
	`sub_type` INT,
	`time` BIGINT,
	primary key(`pid`, `type`, `interact_pid`, `sub_type`)
)ENGINE=MyISAM;