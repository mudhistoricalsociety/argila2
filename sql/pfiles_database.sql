-- MySQL dump 10.7
--
-- Host: localhost    Database: shadows_pfiles
-- ------------------------------------------------------
-- Server version	4.1.3-beta-Max
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE="NO_AUTO_VALUE_ON_ZERO,MYSQL323" */;

--
-- Table structure for table `dreams`
--

DROP TABLE IF EXISTS `dreams`;
CREATE TABLE `dreams` (
  `name` varchar(255) NOT NULL default '',
  `dreamed` int(11) default NULL,
  `dream` text
) TYPE=MyISAM;

--
-- Table structure for table `pfiles`
--

DROP TABLE IF EXISTS `pfiles`;
CREATE TABLE `pfiles` (
  `name` varchar(255) NOT NULL default '',
  `keywords` varchar(255) default NULL,
  `account` varchar(255) default NULL,
  `sdesc` varchar(255) default NULL,
  `ldesc` varchar(255) default NULL,
  `description` text,
  `msg` text,
  `create_comment` text,
  `create_state` int(11) default NULL,
  `nanny_state` int(11) default NULL,
  `role` int(11) default NULL,
  `role_summary` varchar(255) default NULL,
  `role_body` text,
  `role_date` varchar(255) default NULL,
  `role_poster` varchar(75) default NULL,
  `role_cost` int(11) default NULL,
  `app_cost` int(11) default NULL,
  `level` int(11) default NULL,
  `sex` int(11) default NULL,
  `deity` int(11) default NULL,
  `race` int(11) default NULL,
  `room` int(11) default NULL,
  `str` int(11) default NULL,
  `intel` int(11) default NULL,
  `wil` int(11) default NULL,
  `con` int(11) default NULL,
  `dex` int(11) default NULL,
  `aur` int(11) default NULL,
  `agi` int(11) default NULL,
  `start_str` int(11) default NULL,
  `start_intel` int(11) default NULL,
  `start_wil` int(11) default NULL,
  `start_con` int(11) default NULL,
  `start_dex` int(11) default NULL,
  `start_aur` int(11) default NULL,
  `start_agi` int(11) default NULL,
  `played` int(11) default NULL,
  `birth` int(11) default NULL,
  `time` int(11) default NULL,
  `offense` int(11) default NULL,
  `hit` int(11) default NULL,
  `maxhit` int(11) default NULL,
  `nat_attack_type` int(11) default NULL,
  `move` int(11) default NULL,
  `maxmove` int(11) default NULL,
  `circle` int(11) default NULL,
  `ppoints` int(11) default NULL,
  `fightmode` int(11) default NULL,
  `color` int(11) default NULL,
  `speaks` int(11) default NULL,
  `flags` int(11) default NULL,
  `plrflags` int(11) default NULL,
  `boatvnum` int(11) default NULL,
  `speed` int(11) default NULL,
  `mountspeed` int(11) default NULL,
  `sleepneeded` int(11) default NULL,
  `autotoll` int(11) default NULL,
  `coldload` int(10) unsigned NOT NULL default '0',
  `affectedby` int(11) default NULL,
  `affects` text,
  `age` int(11) default NULL,
  `intoxication` int(11) default NULL,
  `hunger` int(11) default NULL,
  `thirst` int(11) default NULL,
  `height` int(11) default NULL,
  `frame` int(11) default NULL,
  `damage` int(11) default NULL,
  `lastregen` int(11) default NULL,
  `lastroom` int(11) default NULL,
  `harness` int(11) default NULL,
  `maxharness` int(11) default NULL,
  `lastlogon` int(11) default NULL,
  `lastlogoff` int(11) default NULL,
  `lastdis` int(11) default NULL,
  `lastconnect` int(11) default NULL,
  `lastdied` int(11) default NULL,
  `hooded` int(11) default NULL,
  `immenter` varchar(255) default NULL,
  `immleave` varchar(255) default NULL,
  `sitelie` varchar(255) default NULL,
  `voicestr` varchar(255) default NULL,
  `clans` text,
  `skills` text,
  `wounds` text,
  `lodged` text,
  `writes` int(11) NOT NULL default '0',
  `profession` int(10) unsigned NOT NULL default '0',
  `was_in_room` int(11) NOT NULL default '0',
  `travelstr` varchar(255) default NULL,
  `last_rpp` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`name`)
) TYPE=MyISAM;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;

