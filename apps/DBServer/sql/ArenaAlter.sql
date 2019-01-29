alter table arena_top_30 add column `date` DATE not null default '2018-07-27' after `rank`;
alter table arena_top_30 drop primary key;
alter table arena_top_30 add primary key (`acc_type`, `acc_id`, `player_id`, `date`);
