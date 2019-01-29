CREATE DATABASE IF NOT EXISTS global DEFAULT CHARSET utf8 COLLATE utf8_general_ci;

USE global;

CREATE TABLE IF NOT EXISTS order_info
(
	`cp_trade_no` VARCHAR(50) PRIMARY KEY, 
	`player_id` INT UNSIGNED,
	`total_fee` float(9,2),
	`currency_type` VARCHAR(50),
	`product_id` VARCHAR(50),
	`create_time` VARCHAR(50),
	`trade_time` VARCHAR(50),
	`is_test_order` VARCHAR(50),
	`channel_type` VARCHAR(50),
	`channel_trade_no` VARCHAR(50),
	`channel` VARCHAR(50),
	`gems` VARCHAR(50)
);
