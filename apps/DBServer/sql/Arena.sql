DROP TABLE IF EXISTS arena_top_30;
CREATE TABLE IF NOT EXISTS arena_top_30
(
	`acc_type` VARCHAR(64) NOT NULL,
	`acc_id` VARCHAR(64) NOT NULL,
	`player_id` VARCHAR(64) NOT NULL,
	`rank` INT,
	`date` DATE NOT NULL,
	`game_region` INT NOT NULL,
	PRIMARY KEY (`acc_type`, `acc_id`, `player_id`, `date`, `game_region`)
)ENGINE=MyISAM;
