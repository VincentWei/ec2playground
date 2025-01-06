-- phpMyAdmin SQL Dump
-- version 4.6.6deb5
-- https://www.phpmyadmin.net/
--
-- Host: localhost:3306
-- Generation Time: Oct 25, 2018 at 11:49 PM
-- Server version: 5.7.23-0ubuntu0.18.04.1
-- PHP Version: 7.2.10-0ubuntu0.18.04.1

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
-- SET time_zone = "+00:00";

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `palyground_ec2`
--

-- --------------------------------------------------------

--
-- Table structure for table `users`
--

CREATE TABLE `users` (
  `id` BIGINT UNSIGNED NOT NULL,
  `name` VARCHAR(256) NOT NULL,
  `passwd` VARCHAR(256) NOT NULL,
  `createdAt` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `lastSignIn` DATETIME NOT NULL
) ENGINE=InnoDB;

ALTER TABLE `users`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `name` (`name`),
  ADD KEY `createdAt` (`createdAt`);

--
-- Table structure for table `snippets`
--

CREATE TABLE `snippets` (
  `id` BIGINT UNSIGNED NOT NULL,
  `digest` VARCHAR(32) NOT NULL,
  `userId` BIGINT UNSIGNED NOT NULL,
  `section` VARCHAR(256) NOT NULL,
  `title` VARCHAR(256) NOT NULL,
  `gitlabId` BIGINT UNSIGNED NOT NULL,
  `score` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `nrStars` BIGINT UNSIGNED NOT NULL DEFAULT 0,
  `nrComments` BIGINT UNSIGNED NOT NULL DEFAULT 0,
  `state` VARCHAR(32) NOT NULL DEFAULT '',
  `createdAt` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `updatedAt` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
) ENGINE=InnoDB;

ALTER TABLE `snippets`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `digest` (`digest`),
  ADD KEY `userId` (`userId`),
  ADD KEY `createdAt` (`createdAt`),
  ADD KEY `updatedAt` (`updatedAt`);

--
-- Table structure for table `selectedSnippets`
--

CREATE TABLE `selectedSnippets` (
  `id` BIGINT UNSIGNED NOT NULL,
  `snippetId` BIGINT UNSIGNED NOT NULL,
  `userId` BIGINT UNSIGNED NOT NULL,
  `createdAt` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB;

ALTER TABLE `selectedSnippets`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `snippetId` (`snippetId`),
  ADD KEY `createdAt` (`createdAt`);

--
-- AUTO_INCREMENT for tables
--
ALTER TABLE `users`
  MODIFY `id` int(4) UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=1;

ALTER TABLE `snippets`
  MODIFY `id` bigint(8) UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=1;

ALTER TABLE `selectedSnippets`
  MODIFY `id` bigint(8) UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=1;
