-- MySQL dump 10.7
--
-- Host: localhost    Database: server_logs
-- ------------------------------------------------------
-- Server version	4.1.3-beta-Max
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE="NO_AUTO_VALUE_ON_ZERO,MYSQL323" */;

--
-- Table structure for table `mud`
--

DROP TABLE IF EXISTS `mud`;
CREATE TABLE `mud` (
  `name` varchar(255) default NULL,
  `account` varchar(255) default NULL,
  `switched_into` varchar(255) default NULL,
  `timestamp` int(11) default '0',
  `port` int(11) default '4500',
  `room` int(11) default '-1',
  `guest` tinyint(1) default '0',
  `immortal` tinyint(1) default '0',
  `error` tinyint(1) default '0',
  `command` varchar(255) default NULL,
  `entry` text,
  `sha_hash` varchar(45) default NULL
) TYPE=MyISAM;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;

