-- MySQL dump 10.7
--
-- Host: localhost    Database: shadows
-- ------------------------------------------------------
-- Server version	4.1.3-beta-Max
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE="NO_AUTO_VALUE_ON_ZERO,MYSQL323" */;

--
-- Table structure for table `account_donations`
--

DROP TABLE IF EXISTS `account_donations`;
CREATE TABLE `account_donations` (
  `contributor` varchar(255) default NULL,
  `currency` varchar(255) default NULL,
  `amount` float default NULL,
  `deposited` float default NULL,
  `timestamp` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `account_referrals`
--

DROP TABLE IF EXISTS `account_referrals`;
CREATE TABLE `account_referrals` (
  `account` varchar(255) default NULL,
  `referrer` varchar(255) default NULL,
  `timestamp` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `account_withdrawals`
--

DROP TABLE IF EXISTS `account_withdrawals`;
CREATE TABLE `account_withdrawals` (
  `withdrawer` varchar(255) default NULL,
  `amount` float default NULL,
  `reason` varchar(255) default NULL,
  `timestamp` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `announcements`
--

DROP TABLE IF EXISTS `announcements`;
CREATE TABLE `announcements` (
  `author` varchar(255) default NULL,
  `entry` text,
  `datestamp` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `application_wait_times`
--

DROP TABLE IF EXISTS `application_wait_times`;
CREATE TABLE `application_wait_times` (
  `wait_time` int(10) unsigned NOT NULL default '0'
) TYPE=MyISAM;

--
-- Table structure for table `banned_sites`
--

DROP TABLE IF EXISTS `banned_sites`;
CREATE TABLE `banned_sites` (
  `site` varchar(255) default NULL,
  `banned_by` varchar(255) default NULL,
  `banned_on` int(11) default NULL,
  `banned_until` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `boards`
--

DROP TABLE IF EXISTS `boards`;
CREATE TABLE `boards` (
  `board_name` varchar(75) default NULL,
  `post_number` int(10) unsigned NOT NULL default '0',
  `subject` varchar(75) default NULL,
  `author` varchar(75) default NULL,
  `ooc_date` varchar(75) default NULL,
  `ic_date` varchar(75) default NULL,
  `message` text,
  `timestamp` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `building_log`
--

DROP TABLE IF EXISTS `building_log`;
CREATE TABLE `building_log` (
  `author` varchar(255) default NULL,
  `entry` text,
  `datestamp` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `building_totals`
--

DROP TABLE IF EXISTS `building_totals`;
CREATE TABLE `building_totals` (
  `rooms` int(11) default NULL,
  `objects` int(11) default NULL,
  `mobiles` int(11) default NULL,
  `crafts` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `changed_helpfiles`
--

DROP TABLE IF EXISTS `changed_helpfiles`;
CREATE TABLE `changed_helpfiles` (
  `staffer` varchar(255) default NULL,
  `change_type` varchar(255) default NULL,
  `category` varchar(255) default NULL,
  `name` varchar(255) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `clan_assignments`
--

DROP TABLE IF EXISTS `clan_assignments`;
CREATE TABLE `clan_assignments` (
  `clan_name` varchar(255) default NULL,
  `imm_name` varchar(255) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `code_changelog`
--

DROP TABLE IF EXISTS `code_changelog`;
CREATE TABLE `code_changelog` (
  `file_list` text,
  `history` text,
  `timestamp` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `coding_log`
--

DROP TABLE IF EXISTS `coding_log`;
CREATE TABLE `coding_log` (
  `author` varchar(255) default NULL,
  `entry` text,
  `datestamp` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `contributions`
--

DROP TABLE IF EXISTS `contributions`;
CREATE TABLE `contributions` (
  `name` varchar(255) default NULL,
  `category` varchar(255) default NULL,
  `contributor` varchar(255) default NULL,
  `filename` varchar(255) default NULL,
  `date` int(11) default NULL,
  `uploaded_by` varchar(255) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `copyover_aiming`
--

DROP TABLE IF EXISTS `copyover_aiming`;
CREATE TABLE `copyover_aiming` (
  `aimer_id` int(11) default NULL,
  `target_id` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `copyover_animations`
--

DROP TABLE IF EXISTS `copyover_animations`;
CREATE TABLE `copyover_animations` (
  `animator` int(11) default NULL,
  `target` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `copyover_crafts`
--

DROP TABLE IF EXISTS `copyover_crafts`;
CREATE TABLE `copyover_crafts` (
  `crafter_id` int(11) default NULL,
  `craft` varchar(255) default NULL,
  `phase` int(11) default NULL,
  `timer` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `copyover_delays`
--

DROP TABLE IF EXISTS `copyover_delays`;
CREATE TABLE `copyover_delays` (
  `char_id` int(11) default NULL,
  `delay` int(11) default NULL,
  `delay_type` int(11) default NULL,
  `delay_info1` int(11) default NULL,
  `delay_info2` int(11) default NULL,
  `delay_who` varchar(255) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `copyover_doors`
--

DROP TABLE IF EXISTS `copyover_doors`;
CREATE TABLE `copyover_doors` (
  `room` int(11) default NULL,
  `direction` int(11) default NULL,
  `state` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `copyover_fighting`
--

DROP TABLE IF EXISTS `copyover_fighting`;
CREATE TABLE `copyover_fighting` (
  `fighter_id` int(11) default NULL,
  `target_id` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `copyover_followers`
--

DROP TABLE IF EXISTS `copyover_followers`;
CREATE TABLE `copyover_followers` (
  `follower_id` int(11) default NULL,
  `leader_id` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `copyover_sighted_targets`
--

DROP TABLE IF EXISTS `copyover_sighted_targets`;
CREATE TABLE `copyover_sighted_targets` (
  `sighter_id` int(11) default NULL,
  `target_id` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `copyover_subduers`
--

DROP TABLE IF EXISTS `copyover_subduers`;
CREATE TABLE `copyover_subduers` (
  `subduer` int(11) default NULL,
  `target` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `crafts`
--

DROP TABLE IF EXISTS `crafts`;
CREATE TABLE `crafts` (
  `craft` varchar(255) default NULL,
  `subcraft` varchar(255) default NULL,
  `command` varchar(255) default NULL,
  `opening` smallint(6) default NULL,
  `required_skills` varchar(255) default NULL,
  `clanned` smallint(6) default NULL,
  `race` smallint(6) default NULL,
  `opens_for` varchar(255) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `donation_tracking`
--

DROP TABLE IF EXISTS `donation_tracking`;
CREATE TABLE `donation_tracking` (
  `donations` float default NULL,
  `spent` float default NULL
) TYPE=MyISAM;

--
-- Table structure for table `downloads`
--

DROP TABLE IF EXISTS `downloads`;
CREATE TABLE `downloads` (
  `host` varchar(255) default NULL,
  `timestamp` int(11) default NULL,
  `filename` varchar(255) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `failed_logins`
--

DROP TABLE IF EXISTS `failed_logins`;
CREATE TABLE `failed_logins` (
  `account_name` varchar(255) default NULL,
  `hostname` varchar(255) default NULL,
  `timestamp` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `forum_auth_access`
--

DROP TABLE IF EXISTS `forum_auth_access`;
CREATE TABLE `forum_auth_access` (
  `group_id` mediumint(8) NOT NULL default '0',
  `forum_id` smallint(5) unsigned NOT NULL default '0',
  `auth_view` tinyint(1) NOT NULL default '0',
  `auth_read` tinyint(1) NOT NULL default '0',
  `auth_post` tinyint(1) NOT NULL default '0',
  `auth_reply` tinyint(1) NOT NULL default '0',
  `auth_edit` tinyint(1) NOT NULL default '0',
  `auth_delete` tinyint(1) NOT NULL default '0',
  `auth_sticky` tinyint(1) NOT NULL default '0',
  `auth_announce` tinyint(1) NOT NULL default '0',
  `auth_vote` tinyint(1) NOT NULL default '0',
  `auth_pollcreate` tinyint(1) NOT NULL default '0',
  `auth_attachments` tinyint(1) NOT NULL default '0',
  `auth_mod` tinyint(1) NOT NULL default '0',
  KEY `group_id` (`group_id`),
  KEY `forum_id` (`forum_id`)
) TYPE=MyISAM;

--
-- Table structure for table `forum_banlist`
--

DROP TABLE IF EXISTS `forum_banlist`;
CREATE TABLE `forum_banlist` (
  `ban_id` mediumint(8) unsigned NOT NULL auto_increment,
  `ban_userid` mediumint(8) NOT NULL default '0',
  `ban_ip` varchar(8) NOT NULL default '',
  `ban_email` varchar(255) default NULL,
  PRIMARY KEY  (`ban_id`),
  KEY `ban_ip_user_id` (`ban_ip`,`ban_userid`)
) TYPE=MyISAM;

--
-- Table structure for table `forum_categories`
--

DROP TABLE IF EXISTS `forum_categories`;
CREATE TABLE `forum_categories` (
  `cat_id` mediumint(8) unsigned NOT NULL auto_increment,
  `cat_title` varchar(100) default NULL,
  `cat_order` mediumint(8) unsigned NOT NULL default '0',
  PRIMARY KEY  (`cat_id`),
  KEY `cat_order` (`cat_order`)
) TYPE=MyISAM;

--
-- Table structure for table `forum_config`
--

DROP TABLE IF EXISTS `forum_config`;
CREATE TABLE `forum_config` (
  `config_name` varchar(255) NOT NULL default '',
  `config_value` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`config_name`)
) TYPE=MyISAM;

--
-- Table structure for table `forum_confirm`
--

DROP TABLE IF EXISTS `forum_confirm`;
CREATE TABLE `forum_confirm` (
  `confirm_id` char(32) NOT NULL default '',
  `session_id` char(32) NOT NULL default '',
  `code` char(6) NOT NULL default '',
  PRIMARY KEY  (`session_id`,`confirm_id`)
) TYPE=MyISAM;

--
-- Table structure for table `forum_disallow`
--

DROP TABLE IF EXISTS `forum_disallow`;
CREATE TABLE `forum_disallow` (
  `disallow_id` mediumint(8) unsigned NOT NULL auto_increment,
  `disallow_username` varchar(25) NOT NULL default '',
  PRIMARY KEY  (`disallow_id`)
) TYPE=MyISAM;

--
-- Table structure for table `forum_forum_lock`
--

DROP TABLE IF EXISTS `forum_forum_lock`;
CREATE TABLE `forum_forum_lock` (
  `lock_id` mediumint(8) unsigned NOT NULL auto_increment,
  `forum_id` smallint(5) unsigned NOT NULL default '0',
  `lock_days` smallint(5) unsigned NOT NULL default '0',
  `lock_freq` smallint(5) unsigned NOT NULL default '0',
  PRIMARY KEY  (`lock_id`),
  KEY `forum_id` (`forum_id`)
) TYPE=MyISAM;

--
-- Table structure for table `forum_forum_prune`
--

DROP TABLE IF EXISTS `forum_forum_prune`;
CREATE TABLE `forum_forum_prune` (
  `prune_id` mediumint(8) unsigned NOT NULL auto_increment,
  `forum_id` smallint(5) unsigned NOT NULL default '0',
  `prune_days` smallint(5) unsigned NOT NULL default '0',
  `prune_freq` smallint(5) unsigned NOT NULL default '0',
  PRIMARY KEY  (`prune_id`),
  KEY `forum_id` (`forum_id`)
) TYPE=MyISAM;

--
-- Table structure for table `forum_forums`
--

DROP TABLE IF EXISTS `forum_forums`;
CREATE TABLE `forum_forums` (
  `forum_id` smallint(5) unsigned NOT NULL default '0',
  `cat_id` mediumint(8) unsigned NOT NULL default '0',
  `forum_name` varchar(150) default NULL,
  `forum_desc` text,
  `forum_status` tinyint(4) NOT NULL default '0',
  `forum_order` mediumint(8) unsigned NOT NULL default '1',
  `forum_posts` mediumint(8) unsigned NOT NULL default '0',
  `forum_topics` mediumint(8) unsigned NOT NULL default '0',
  `forum_last_post_id` mediumint(8) unsigned NOT NULL default '0',
  `prune_next` int(11) default NULL,
  `prune_enable` tinyint(1) NOT NULL default '0',
  `auth_view` tinyint(2) NOT NULL default '0',
  `auth_read` tinyint(2) NOT NULL default '0',
  `auth_post` tinyint(2) NOT NULL default '0',
  `auth_reply` tinyint(2) NOT NULL default '0',
  `auth_edit` tinyint(2) NOT NULL default '0',
  `auth_delete` tinyint(2) NOT NULL default '0',
  `auth_sticky` tinyint(2) NOT NULL default '0',
  `auth_announce` tinyint(2) NOT NULL default '0',
  `auth_vote` tinyint(2) NOT NULL default '0',
  `auth_pollcreate` tinyint(2) NOT NULL default '0',
  `auth_attachments` tinyint(2) NOT NULL default '0',
  `forum_clan` varchar(255) default NULL,
  `lock_enable` tinyint(1) NOT NULL default '0',
  `lock_next` int(11) default NULL,
  PRIMARY KEY  (`forum_id`),
  KEY `forums_order` (`forum_order`),
  KEY `cat_id` (`cat_id`),
  KEY `forum_last_post_id` (`forum_last_post_id`),
  KEY `forum_id` (`forum_id`)
) TYPE=MyISAM;

--
-- Table structure for table `forum_groups`
--

DROP TABLE IF EXISTS `forum_groups`;
CREATE TABLE `forum_groups` (
  `group_id` mediumint(8) NOT NULL auto_increment,
  `group_type` tinyint(4) NOT NULL default '1',
  `group_name` varchar(40) NOT NULL default '',
  `group_description` varchar(255) NOT NULL default '',
  `group_moderator` mediumint(8) NOT NULL default '0',
  `group_single_user` tinyint(1) NOT NULL default '1',
  PRIMARY KEY  (`group_id`),
  KEY `group_single_user` (`group_single_user`)
) TYPE=MyISAM;

--
-- Table structure for table `forum_posts`
--

DROP TABLE IF EXISTS `forum_posts`;
CREATE TABLE `forum_posts` (
  `post_id` mediumint(8) unsigned NOT NULL auto_increment,
  `topic_id` mediumint(8) unsigned NOT NULL default '0',
  `forum_id` smallint(5) unsigned NOT NULL default '0',
  `poster_id` mediumint(8) NOT NULL default '0',
  `post_time` int(11) NOT NULL default '0',
  `poster_ip` varchar(8) NOT NULL default '',
  `post_username` varchar(25) default NULL,
  `enable_bbcode` tinyint(1) NOT NULL default '1',
  `enable_html` tinyint(1) NOT NULL default '0',
  `enable_smilies` tinyint(1) NOT NULL default '1',
  `enable_sig` tinyint(1) NOT NULL default '1',
  `post_edit_time` int(11) default NULL,
  `post_edit_count` smallint(5) unsigned NOT NULL default '0',
  PRIMARY KEY  (`post_id`),
  KEY `forum_id` (`forum_id`),
  KEY `topic_id` (`topic_id`),
  KEY `poster_id` (`poster_id`),
  KEY `post_time` (`post_time`)
) TYPE=MyISAM;

--
-- Table structure for table `forum_posts_text`
--

DROP TABLE IF EXISTS `forum_posts_text`;
CREATE TABLE `forum_posts_text` (
  `post_id` mediumint(8) unsigned NOT NULL default '0',
  `bbcode_uid` varchar(10) NOT NULL default '',
  `post_subject` varchar(60) default NULL,
  `post_text` text,
  PRIMARY KEY  (`post_id`)
) TYPE=MyISAM;

--
-- Table structure for table `forum_privmsgs`
--

DROP TABLE IF EXISTS `forum_privmsgs`;
CREATE TABLE `forum_privmsgs` (
  `privmsgs_id` mediumint(8) unsigned NOT NULL auto_increment,
  `privmsgs_type` tinyint(4) NOT NULL default '0',
  `privmsgs_subject` varchar(255) NOT NULL default '0',
  `privmsgs_from_userid` mediumint(8) NOT NULL default '0',
  `privmsgs_to_userid` mediumint(8) NOT NULL default '0',
  `privmsgs_date` int(11) NOT NULL default '0',
  `privmsgs_ip` varchar(8) NOT NULL default '',
  `privmsgs_enable_bbcode` tinyint(1) NOT NULL default '1',
  `privmsgs_enable_html` tinyint(1) NOT NULL default '0',
  `privmsgs_enable_smilies` tinyint(1) NOT NULL default '1',
  `privmsgs_attach_sig` tinyint(1) NOT NULL default '1',
  PRIMARY KEY  (`privmsgs_id`),
  KEY `privmsgs_from_userid` (`privmsgs_from_userid`),
  KEY `privmsgs_to_userid` (`privmsgs_to_userid`)
) TYPE=MyISAM;

--
-- Table structure for table `forum_privmsgs_text`
--

DROP TABLE IF EXISTS `forum_privmsgs_text`;
CREATE TABLE `forum_privmsgs_text` (
  `privmsgs_text_id` mediumint(8) unsigned NOT NULL default '0',
  `privmsgs_bbcode_uid` varchar(10) NOT NULL default '0',
  `privmsgs_text` text,
  PRIMARY KEY  (`privmsgs_text_id`)
) TYPE=MyISAM;

--
-- Table structure for table `forum_ranks`
--

DROP TABLE IF EXISTS `forum_ranks`;
CREATE TABLE `forum_ranks` (
  `rank_id` smallint(5) unsigned NOT NULL auto_increment,
  `rank_title` varchar(50) NOT NULL default '',
  `rank_min` mediumint(8) NOT NULL default '0',
  `rank_special` tinyint(1) default '0',
  `rank_image` varchar(255) default NULL,
  PRIMARY KEY  (`rank_id`)
) TYPE=MyISAM;

--
-- Table structure for table `forum_search_results`
--

DROP TABLE IF EXISTS `forum_search_results`;
CREATE TABLE `forum_search_results` (
  `search_id` int(11) unsigned NOT NULL default '0',
  `session_id` varchar(32) NOT NULL default '',
  `search_array` text NOT NULL,
  PRIMARY KEY  (`search_id`),
  KEY `session_id` (`session_id`)
) TYPE=MyISAM;

--
-- Table structure for table `forum_search_wordlist`
--

DROP TABLE IF EXISTS `forum_search_wordlist`;
CREATE TABLE `forum_search_wordlist` (
  `word_text` varchar(50) NOT NULL default '',
  `word_id` mediumint(8) unsigned NOT NULL auto_increment,
  `word_common` tinyint(1) unsigned NOT NULL default '0',
  PRIMARY KEY  (`word_text`),
  KEY `word_id` (`word_id`)
) TYPE=MyISAM;

--
-- Table structure for table `forum_search_wordmatch`
--

DROP TABLE IF EXISTS `forum_search_wordmatch`;
CREATE TABLE `forum_search_wordmatch` (
  `post_id` mediumint(8) unsigned NOT NULL default '0',
  `word_id` mediumint(8) unsigned NOT NULL default '0',
  `title_match` tinyint(1) NOT NULL default '0',
  KEY `post_id` (`post_id`),
  KEY `word_id` (`word_id`)
) TYPE=MyISAM;

--
-- Table structure for table `forum_sessions`
--

DROP TABLE IF EXISTS `forum_sessions`;
CREATE TABLE `forum_sessions` (
  `session_id` char(32) NOT NULL default '',
  `session_user_id` mediumint(8) NOT NULL default '0',
  `session_start` int(11) NOT NULL default '0',
  `session_time` int(11) NOT NULL default '0',
  `session_ip` char(8) NOT NULL default '0',
  `session_page` int(11) NOT NULL default '0',
  `session_logged_in` tinyint(1) NOT NULL default '0',
  PRIMARY KEY  (`session_id`),
  KEY `session_user_id` (`session_user_id`),
  KEY `session_id_ip_user_id` (`session_id`,`session_ip`,`session_user_id`)
) TYPE=MyISAM;

--
-- Table structure for table `forum_smilies`
--

DROP TABLE IF EXISTS `forum_smilies`;
CREATE TABLE `forum_smilies` (
  `smilies_id` smallint(5) unsigned NOT NULL auto_increment,
  `code` varchar(50) default NULL,
  `smile_url` varchar(100) default NULL,
  `emoticon` varchar(75) default NULL,
  PRIMARY KEY  (`smilies_id`)
) TYPE=MyISAM;

--
-- Table structure for table `forum_themes`
--

DROP TABLE IF EXISTS `forum_themes`;
CREATE TABLE `forum_themes` (
  `themes_id` mediumint(8) unsigned NOT NULL auto_increment,
  `template_name` varchar(30) NOT NULL default '',
  `style_name` varchar(30) NOT NULL default '',
  `head_stylesheet` varchar(100) default NULL,
  `body_background` varchar(100) default NULL,
  `body_bgcolor` varchar(6) default NULL,
  `body_text` varchar(6) default NULL,
  `body_link` varchar(6) default NULL,
  `body_vlink` varchar(6) default NULL,
  `body_alink` varchar(6) default NULL,
  `body_hlink` varchar(6) default NULL,
  `tr_color1` varchar(6) default NULL,
  `tr_color2` varchar(6) default NULL,
  `tr_color3` varchar(6) default NULL,
  `tr_class1` varchar(25) default NULL,
  `tr_class2` varchar(25) default NULL,
  `tr_class3` varchar(25) default NULL,
  `th_color1` varchar(6) default NULL,
  `th_color2` varchar(6) default NULL,
  `th_color3` varchar(6) default NULL,
  `th_class1` varchar(25) default NULL,
  `th_class2` varchar(25) default NULL,
  `th_class3` varchar(25) default NULL,
  `td_color1` varchar(6) default NULL,
  `td_color2` varchar(6) default NULL,
  `td_color3` varchar(6) default NULL,
  `td_class1` varchar(25) default NULL,
  `td_class2` varchar(25) default NULL,
  `td_class3` varchar(25) default NULL,
  `fontface1` varchar(50) default NULL,
  `fontface2` varchar(50) default NULL,
  `fontface3` varchar(50) default NULL,
  `fontsize1` tinyint(4) default NULL,
  `fontsize2` tinyint(4) default NULL,
  `fontsize3` tinyint(4) default NULL,
  `fontcolor1` varchar(6) default NULL,
  `fontcolor2` varchar(6) default NULL,
  `fontcolor3` varchar(6) default NULL,
  `span_class1` varchar(25) default NULL,
  `span_class2` varchar(25) default NULL,
  `span_class3` varchar(25) default NULL,
  `img_size_poll` smallint(5) unsigned default NULL,
  `img_size_privmsg` smallint(5) unsigned default NULL,
  PRIMARY KEY  (`themes_id`)
) TYPE=MyISAM;

--
-- Table structure for table `forum_themes_name`
--

DROP TABLE IF EXISTS `forum_themes_name`;
CREATE TABLE `forum_themes_name` (
  `themes_id` smallint(5) unsigned NOT NULL default '0',
  `tr_color1_name` char(50) default NULL,
  `tr_color2_name` char(50) default NULL,
  `tr_color3_name` char(50) default NULL,
  `tr_class1_name` char(50) default NULL,
  `tr_class2_name` char(50) default NULL,
  `tr_class3_name` char(50) default NULL,
  `th_color1_name` char(50) default NULL,
  `th_color2_name` char(50) default NULL,
  `th_color3_name` char(50) default NULL,
  `th_class1_name` char(50) default NULL,
  `th_class2_name` char(50) default NULL,
  `th_class3_name` char(50) default NULL,
  `td_color1_name` char(50) default NULL,
  `td_color2_name` char(50) default NULL,
  `td_color3_name` char(50) default NULL,
  `td_class1_name` char(50) default NULL,
  `td_class2_name` char(50) default NULL,
  `td_class3_name` char(50) default NULL,
  `fontface1_name` char(50) default NULL,
  `fontface2_name` char(50) default NULL,
  `fontface3_name` char(50) default NULL,
  `fontsize1_name` char(50) default NULL,
  `fontsize2_name` char(50) default NULL,
  `fontsize3_name` char(50) default NULL,
  `fontcolor1_name` char(50) default NULL,
  `fontcolor2_name` char(50) default NULL,
  `fontcolor3_name` char(50) default NULL,
  `span_class1_name` char(50) default NULL,
  `span_class2_name` char(50) default NULL,
  `span_class3_name` char(50) default NULL,
  PRIMARY KEY  (`themes_id`)
) TYPE=MyISAM;

--
-- Table structure for table `forum_topics`
--

DROP TABLE IF EXISTS `forum_topics`;
CREATE TABLE `forum_topics` (
  `topic_id` mediumint(8) unsigned NOT NULL auto_increment,
  `forum_id` smallint(8) unsigned NOT NULL default '0',
  `topic_title` char(60) NOT NULL default '',
  `topic_poster` mediumint(8) NOT NULL default '0',
  `topic_time` int(11) NOT NULL default '0',
  `topic_views` mediumint(8) unsigned NOT NULL default '0',
  `topic_replies` mediumint(8) unsigned NOT NULL default '0',
  `topic_status` tinyint(3) NOT NULL default '0',
  `topic_vote` tinyint(1) NOT NULL default '0',
  `topic_type` tinyint(3) NOT NULL default '0',
  `topic_first_post_id` mediumint(8) unsigned NOT NULL default '0',
  `topic_last_post_id` mediumint(8) unsigned NOT NULL default '0',
  `topic_moved_id` mediumint(8) unsigned NOT NULL default '0',
  PRIMARY KEY  (`topic_id`),
  KEY `forum_id` (`forum_id`),
  KEY `topic_moved_id` (`topic_moved_id`),
  KEY `topic_status` (`topic_status`),
  KEY `topic_type` (`topic_type`)
) TYPE=MyISAM;

--
-- Table structure for table `forum_topics_watch`
--

DROP TABLE IF EXISTS `forum_topics_watch`;
CREATE TABLE `forum_topics_watch` (
  `topic_id` mediumint(8) unsigned NOT NULL default '0',
  `user_id` mediumint(8) NOT NULL default '0',
  `notify_status` tinyint(1) NOT NULL default '0',
  KEY `topic_id` (`topic_id`),
  KEY `user_id` (`user_id`),
  KEY `notify_status` (`notify_status`)
) TYPE=MyISAM;

--
-- Table structure for table `forum_user_group`
--

DROP TABLE IF EXISTS `forum_user_group`;
CREATE TABLE `forum_user_group` (
  `group_id` mediumint(8) NOT NULL default '0',
  `user_id` mediumint(8) NOT NULL default '0',
  `user_pending` tinyint(1) default NULL,
  KEY `group_id` (`group_id`),
  KEY `user_id` (`user_id`)
) TYPE=MyISAM;

--
-- Table structure for table `forum_users`
--

DROP TABLE IF EXISTS `forum_users`;
CREATE TABLE `forum_users` (
  `user_id` mediumint(8) NOT NULL default '0',
  `user_active` tinyint(1) default '1',
  `username` varchar(25) NOT NULL default '',
  `roleplay_points` int(10) unsigned default '0',
  `user_password` varchar(32) NOT NULL default '',
  `user_last_ip` varchar(80) default NULL,
  `user_subscription` smallint(5) unsigned default '1',
  `user_color` smallint(5) unsigned default '0',
  `user_session_time` int(11) NOT NULL default '0',
  `user_session_page` smallint(5) NOT NULL default '0',
  `user_lastvisit` int(11) NOT NULL default '0',
  `user_regdate` int(11) NOT NULL default '0',
  `user_level` tinyint(4) default '0',
  `user_posts` mediumint(8) unsigned NOT NULL default '0',
  `user_timezone` decimal(5,2) NOT NULL default '0.00',
  `user_style` tinyint(4) default NULL,
  `user_lang` varchar(255) default NULL,
  `user_dateformat` varchar(14) NOT NULL default 'd M Y H:i',
  `user_new_privmsg` smallint(5) unsigned NOT NULL default '0',
  `user_unread_privmsg` smallint(5) unsigned NOT NULL default '0',
  `user_last_privmsg` int(11) NOT NULL default '0',
  `user_emailtime` int(11) default NULL,
  `user_viewemail` tinyint(1) default NULL,
  `user_attachsig` tinyint(1) default NULL,
  `user_allowhtml` tinyint(1) default '1',
  `user_allowbbcode` tinyint(1) default '1',
  `user_allowsmile` tinyint(1) default '1',
  `user_allowavatar` tinyint(1) NOT NULL default '1',
  `user_allow_pm` tinyint(1) NOT NULL default '1',
  `user_allow_viewonline` tinyint(1) NOT NULL default '1',
  `user_notify` tinyint(1) NOT NULL default '1',
  `user_notify_pm` tinyint(1) NOT NULL default '0',
  `user_popup_pm` tinyint(1) NOT NULL default '0',
  `user_rank` int(11) default '0',
  `user_avatar` varchar(100) default NULL,
  `user_avatar_type` tinyint(4) NOT NULL default '0',
  `user_email` varchar(255) default NULL,
  `user_icq` varchar(15) default NULL,
  `user_website` varchar(100) default NULL,
  `user_from` varchar(100) default NULL,
  `user_sig` text,
  `user_sig_bbcode_uid` varchar(10) default NULL,
  `user_aim` varchar(255) default NULL,
  `user_yim` varchar(255) default NULL,
  `user_msnm` varchar(255) default NULL,
  `user_occ` varchar(100) default NULL,
  `user_interests` varchar(255) default NULL,
  `user_actkey` varchar(32) default NULL,
  `user_newpasswd` varchar(32) default NULL,
  `account_flags` int(11) NOT NULL default '0',
  `tms_votes` int(10) unsigned default '0',
  `last_tms_vote` int(10) unsigned default '0',
  `mm_votes` int(10) unsigned NOT NULL default '0',
  `last_mm_vote` int(10) unsigned NOT NULL default '0',
  `tmc_votes` int(10) unsigned NOT NULL default '0',
  `last_tmc_vote` int(10) unsigned NOT NULL default '0',
  `downloaded_code` int(10) unsigned default NULL,
  `last_rpp` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`user_id`),
  KEY `user_session_time` (`user_session_time`)
) TYPE=MyISAM;

--
-- Table structure for table `forum_vote_desc`
--

DROP TABLE IF EXISTS `forum_vote_desc`;
CREATE TABLE `forum_vote_desc` (
  `vote_id` mediumint(8) unsigned NOT NULL auto_increment,
  `topic_id` mediumint(8) unsigned NOT NULL default '0',
  `vote_text` text NOT NULL,
  `vote_start` int(11) NOT NULL default '0',
  `vote_length` int(11) NOT NULL default '0',
  PRIMARY KEY  (`vote_id`),
  KEY `topic_id` (`topic_id`)
) TYPE=MyISAM;

--
-- Table structure for table `forum_vote_results`
--

DROP TABLE IF EXISTS `forum_vote_results`;
CREATE TABLE `forum_vote_results` (
  `vote_id` mediumint(8) unsigned NOT NULL default '0',
  `vote_option_id` tinyint(4) unsigned NOT NULL default '0',
  `vote_option_text` varchar(255) NOT NULL default '',
  `vote_result` int(11) NOT NULL default '0',
  KEY `vote_option_id` (`vote_option_id`),
  KEY `vote_id` (`vote_id`)
) TYPE=MyISAM;

--
-- Table structure for table `forum_vote_voters`
--

DROP TABLE IF EXISTS `forum_vote_voters`;
CREATE TABLE `forum_vote_voters` (
  `vote_id` mediumint(8) unsigned NOT NULL default '0',
  `vote_user_id` mediumint(8) NOT NULL default '0',
  `vote_user_ip` char(8) NOT NULL default '',
  KEY `vote_id` (`vote_id`),
  KEY `vote_user_id` (`vote_user_id`),
  KEY `vote_user_ip` (`vote_user_ip`)
) TYPE=MyISAM;

--
-- Table structure for table `forum_words`
--

DROP TABLE IF EXISTS `forum_words`;
CREATE TABLE `forum_words` (
  `word_id` mediumint(8) unsigned NOT NULL auto_increment,
  `word` char(100) NOT NULL default '',
  `replacement` char(100) NOT NULL default '',
  PRIMARY KEY  (`word_id`)
) TYPE=MyISAM;

--
-- Table structure for table `helpfiles`
--

DROP TABLE IF EXISTS `helpfiles`;
CREATE TABLE `helpfiles` (
  `name` varchar(250) NOT NULL default '',
  `category` varchar(250) NOT NULL default '',
  `entry` text,
  `related_entries` varchar(250) default NULL,
  `required_level` int(10) unsigned default NULL,
  `last_changed` varchar(255) default NULL,
  `changed_by` varchar(255) default NULL,
  PRIMARY KEY  (`name`,`category`)
) TYPE=MyISAM;

--
-- Table structure for table `hobbitmail`
--

DROP TABLE IF EXISTS `hobbitmail`;
CREATE TABLE `hobbitmail` (
  `account` varchar(255) default NULL,
  `flags` int(10) unsigned default '0',
  `from_line` varchar(255) default NULL,
  `from_account` varchar(255) default NULL,
  `sent_date` varchar(255) default NULL,
  `subject` varchar(255) default NULL,
  `message` text,
  `timestamp` int(10) unsigned NOT NULL default '0',
  `id` int(11) NOT NULL auto_increment,
  PRIMARY KEY  (`id`)
) TYPE=MyISAM;

--
-- Table structure for table `massmail_messages`
--

DROP TABLE IF EXISTS `massmail_messages`;
CREATE TABLE `massmail_messages` (
  `subject` varchar(255) default NULL,
  `message` text,
  `username` varchar(255) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `mob_resets`
--

DROP TABLE IF EXISTS `mob_resets`;
CREATE TABLE `mob_resets` (
  `zone` int(11) default NULL,
  `cmd_no` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `new_crafts`
--

DROP TABLE IF EXISTS `new_crafts`;
CREATE TABLE `new_crafts` (
  `command` varchar(255) default NULL,
  `subcraft` varchar(255) default NULL,
  `craft` varchar(255) default NULL,
  `creator` varchar(255) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `newsletter_issue`
--

DROP TABLE IF EXISTS `newsletter_issue`;
CREATE TABLE `newsletter_issue` (
  `volume` int(11) default NULL,
  `issue` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `newsletter_stats`
--

DROP TABLE IF EXISTS `newsletter_stats`;
CREATE TABLE `newsletter_stats` (
  `accepted_apps` int(11) NOT NULL default '0',
  `declined_apps` int(11) NOT NULL default '0',
  `resolved_typos` int(11) NOT NULL default '0',
  `resolved_bugs` int(11) NOT NULL default '0',
  `pc_deaths` int(11) NOT NULL default '0',
  `new_accounts` int(11) NOT NULL default '0',
  `guest_logins` int(11) NOT NULL default '0',
  `most_online` int(11) NOT NULL default '0'
) TYPE=MyISAM;

--
-- Table structure for table `newsletters`
--

DROP TABLE IF EXISTS `newsletters`;
CREATE TABLE `newsletters` (
  `timestamp` int(11) default NULL,
  `newsletter` text
) TYPE=MyISAM;

--
-- Table structure for table `player_journals`
--

DROP TABLE IF EXISTS `player_journals`;
CREATE TABLE `player_journals` (
  `name` varchar(75) default NULL,
  `post_number` int(10) unsigned NOT NULL default '0',
  `subject` varchar(75) default NULL,
  `author` varchar(75) default NULL,
  `date` varchar(75) default NULL,
  `message` text
) TYPE=MyISAM;

--
-- Table structure for table `player_notes`
--

DROP TABLE IF EXISTS `player_notes`;
CREATE TABLE `player_notes` (
  `name` varchar(75) default NULL,
  `post_number` int(10) unsigned NOT NULL default '0',
  `subject` varchar(75) default NULL,
  `author` varchar(75) default NULL,
  `date` varchar(75) default NULL,
  `message` text,
  `flags` int(11) default NULL,
  `timestamp` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `player_writing`
--

DROP TABLE IF EXISTS `player_writing`;
CREATE TABLE `player_writing` (
  `db_key` int(11) default NULL,
  `author` varchar(255) default NULL,
  `page` int(11) default NULL,
  `date` varchar(255) default NULL,
  `ink` varchar(255) default NULL,
  `language` int(11) default NULL,
  `script` int(11) default NULL,
  `skill` int(11) default NULL,
  `writing` text,
  `timestamp` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `players_online`
--

DROP TABLE IF EXISTS `players_online`;
CREATE TABLE `players_online` (
  `account` varchar(255) default NULL,
  `name` varchar(255) default NULL,
  `hostname` varchar(255) default NULL,
  `room` int(11) default NULL,
  `port` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `plot_log`
--

DROP TABLE IF EXISTS `plot_log`;
CREATE TABLE `plot_log` (
  `author` varchar(255) default NULL,
  `entry` text,
  `datestamp` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `professions`
--

DROP TABLE IF EXISTS `professions`;
CREATE TABLE `professions` (
  `name` varchar(255) default NULL,
  `skill_list` varchar(255) default NULL,
  `id` int(10) unsigned NOT NULL auto_increment,
  `picked` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`)
) TYPE=MyISAM;

--
-- Table structure for table `queued_reviews`
--

DROP TABLE IF EXISTS `queued_reviews`;
CREATE TABLE `queued_reviews` (
  `char_name` varchar(255) default NULL,
  `reviewer` varchar(255) default NULL,
  `new_keywords` varchar(255) default NULL,
  `new_sdesc` varchar(255) default NULL,
  `new_ldesc` varchar(255) default NULL,
  `new_desc` text,
  `new_comment` text,
  `accepted` smallint(5) unsigned default '0',
  `response` text
) TYPE=MyISAM;

--
-- Table structure for table `queued_startups`
--

DROP TABLE IF EXISTS `queued_startups`;
CREATE TABLE `queued_startups` (
  `port` varchar(255) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `queued_submissions`
--

DROP TABLE IF EXISTS `queued_submissions`;
CREATE TABLE `queued_submissions` (
  `account` varchar(30) default NULL,
  `type` varchar(30) default NULL,
  `field1` text,
  `field2` text,
  `field3` text,
  `field4` text,
  `field5` text,
  `field6` text,
  `field7` text,
  `field8` text,
  `id` int(10) unsigned NOT NULL auto_increment,
  PRIMARY KEY  (`id`)
) TYPE=MyISAM;

--
-- Table structure for table `race_rpp_data`
--

DROP TABLE IF EXISTS `race_rpp_data`;
CREATE TABLE `race_rpp_data` (
  `race_name` varchar(255) default NULL,
  `point_req` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `reboot_mobiles`
--

DROP TABLE IF EXISTS `reboot_mobiles`;
CREATE TABLE `reboot_mobiles` (
  `vnum` int(11) default NULL,
  `room` int(11) default NULL,
  `coldload_id` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `registered_characters`
--

DROP TABLE IF EXISTS `registered_characters`;
CREATE TABLE `registered_characters` (
  `account_name` varchar(255) default NULL,
  `character_name` varchar(255) default NULL,
  `regdate` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `resolved_hosts`
--

DROP TABLE IF EXISTS `resolved_hosts`;
CREATE TABLE `resolved_hosts` (
  `ip` varchar(255) default NULL,
  `hostname` varchar(255) default NULL,
  `timestamp` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `reviews_in_progress`
--

DROP TABLE IF EXISTS `reviews_in_progress`;
CREATE TABLE `reviews_in_progress` (
  `char_name` varchar(255) default NULL,
  `reviewer` varchar(255) default NULL,
  `timestamp` int(10) unsigned default '0'
) TYPE=MyISAM;

--
-- Table structure for table `saveroom_objs`
--

DROP TABLE IF EXISTS `saveroom_objs`;
CREATE TABLE `saveroom_objs` (
  `room` int(11) default NULL,
  `wc` int(11) default NULL,
  `pos` int(11) default NULL,
  `objstack` int(11) default NULL,
  `obj_vnum` int(11) default NULL,
  `name` varchar(255) default NULL,
  `short_desc` varchar(255) default NULL,
  `long_desc` varchar(255) default NULL,
  `full_desc` text,
  `desc_keys` varchar(255) default NULL,
  `book_title` varchar(255) default NULL,
  `title_skill` int(11) default NULL,
  `title_language` int(11) default NULL,
  `title_script` int(11) default NULL,
  `loaded` int(11) default NULL,
  `extraflags` int(11) default NULL,
  `oval0` int(11) default NULL,
  `oval1` int(11) default NULL,
  `oval2` int(11) default NULL,
  `oval3` int(11) default NULL,
  `oval4` int(11) default NULL,
  `oval5` int(11) default NULL,
  `weight` int(11) default NULL,
  `size` int(11) default NULL,
  `count` int(11) default NULL,
  `timer` int(11) default NULL,
  `clock` int(11) default NULL,
  `morphto` int(11) default NULL,
  `morphTime` int(11) default NULL,
  `varcolor` varchar(255) default NULL,
  `omotestr` varchar(255) default NULL,
  `wounds` text,
  `lodged` text,
  `affects` text
) TYPE=MyISAM;

--
-- Table structure for table `server_statistics`
--

DROP TABLE IF EXISTS `server_statistics`;
CREATE TABLE `server_statistics` (
  `last_crash` int(11) default NULL,
  `last_reboot` int(11) default NULL,
  `max_players` int(11) default NULL,
  `morgul_arena` int(10) unsigned NOT NULL default '0'
) TYPE=MyISAM;

--
-- Table structure for table `special_roles`
--

DROP TABLE IF EXISTS `special_roles`;
CREATE TABLE `special_roles` (
  `summary` varchar(255) default NULL,
  `poster` varchar(255) default NULL,
  `date` varchar(255) default NULL,
  `cost` int(10) unsigned default NULL,
  `body` text,
  `timestamp` int(10) unsigned NOT NULL default '0'
) TYPE=MyISAM;

--
-- Table structure for table `spells`
--

DROP TABLE IF EXISTS `spells`;
CREATE TABLE `spells` (
  `id` int(11) NOT NULL auto_increment,
  `name` varchar(255) default NULL,
  `base_cost` int(11) default NULL,
  `sphere` smallint(6) default NULL,
  `circle` smallint(6) default NULL,
  `ch_echo` varchar(255) default NULL,
  `self_echo` varchar(255) default NULL,
  `vict_echo` varchar(255) default NULL,
  `room_echo` varchar(255) default NULL,
  `spell_description` text,
  `target_type` int(11) default NULL,
  `save_type` int(11) default NULL,
  `save_for` int(11) default NULL,
  `damage_form` int(11) default NULL,
  `affect1` int(11) default NULL,
  `dice1` varchar(20) default NULL,
  `act1` varchar(255) default NULL,
  `fade1` varchar(255) default NULL,
  `affect2` int(11) default NULL,
  `dice2` varchar(20) default NULL,
  `act2` varchar(255) default NULL,
  `fade2` varchar(255) default NULL,
  `affect3` int(11) default NULL,
  `dice3` varchar(20) default NULL,
  `act3` varchar(255) default NULL,
  `fade3` varchar(255) default NULL,
  `affect4` int(11) default NULL,
  `dice4` varchar(20) default NULL,
  `act4` varchar(255) default NULL,
  `fade4` varchar(255) default NULL,
  `affect5` int(11) default NULL,
  `dice5` varchar(20) default NULL,
  `act5` varchar(255) default NULL,
  `fade5` varchar(255) default NULL,
  `author` varchar(255) default NULL,
  `last_modified` int(10) unsigned default NULL,
  PRIMARY KEY  (`id`)
) TYPE=MyISAM;

--
-- Table structure for table `staff_roster`
--

DROP TABLE IF EXISTS `staff_roster`;
CREATE TABLE `staff_roster` (
  `name` varchar(255) default NULL,
  `title` varchar(255) default NULL,
  `seniority` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `stayput_mobiles`
--

DROP TABLE IF EXISTS `stayput_mobiles`;
CREATE TABLE `stayput_mobiles` (
  `vnum` int(11) default NULL,
  `room` int(11) default NULL,
  `coldload_id` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `testimonials`
--

DROP TABLE IF EXISTS `testimonials`;
CREATE TABLE `testimonials` (
  `quote` text,
  `name` varchar(255) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `tracks`
--

DROP TABLE IF EXISTS `tracks`;
CREATE TABLE `tracks` (
  `room` int(10) unsigned default NULL,
  `race` int(10) unsigned default NULL,
  `from_dir` int(10) unsigned default NULL,
  `to_dir` int(10) unsigned default NULL,
  `hours_passed` int(10) unsigned default NULL,
  `speed` int(10) unsigned default NULL,
  `flags` int(10) unsigned default NULL
) TYPE=MyISAM;

--
-- Table structure for table `unneeded_helpfiles`
--

DROP TABLE IF EXISTS `unneeded_helpfiles`;
CREATE TABLE `unneeded_helpfiles` (
  `name` varchar(255) default NULL,
  `datestamp` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `virtual_boards`
--

DROP TABLE IF EXISTS `virtual_boards`;
CREATE TABLE `virtual_boards` (
  `board_name` varchar(75) NOT NULL default '',
  `post_number` int(10) unsigned NOT NULL default '0',
  `subject` varchar(75) default NULL,
  `author` varchar(75) default NULL,
  `date_posted` varchar(75) default NULL,
  `message` text,
  `timestamp` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `vote_notifications`
--

DROP TABLE IF EXISTS `vote_notifications`;
CREATE TABLE `vote_notifications` (
  `ip_addr` varchar(255) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `vote_tracking`
--

DROP TABLE IF EXISTS `vote_tracking`;
CREATE TABLE `vote_tracking` (
  `ip_address` varchar(255) default NULL,
  `site_id` varchar(10) default NULL,
  `datestamp` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `website_links`
--

DROP TABLE IF EXISTS `website_links`;
CREATE TABLE `website_links` (
  `url` varchar(255) default NULL,
  `name` varchar(255) default NULL,
  `category` varchar(255) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `website_log`
--

DROP TABLE IF EXISTS `website_log`;
CREATE TABLE `website_log` (
  `author` varchar(255) default NULL,
  `entry` text,
  `datestamp` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `website_referrals`
--

DROP TABLE IF EXISTS `website_referrals`;
CREATE TABLE `website_referrals` (
  `site` varchar(255) default NULL,
  `timestamp` int(11) default NULL
) TYPE=MyISAM;

--
-- Table structure for table `website_sessions`
--

DROP TABLE IF EXISTS `website_sessions`;
CREATE TABLE `website_sessions` (
  `username` varchar(255) default NULL,
  `random_string` varchar(255) default NULL,
  `staff` smallint(6) NOT NULL default '0'
) TYPE=MyISAM;

--
-- Table structure for table `wound_data`
--

DROP TABLE IF EXISTS `wound_data`;
CREATE TABLE `wound_data` (
  `severity` varchar(255) default NULL,
  `points` int(11) default NULL,
  `time_to_heal` int(11) default NULL,
  `infected` int(11) default NULL,
  `constitution` int(11) default NULL
) TYPE=MyISAM;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;

