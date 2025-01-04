-- phpMyAdmin SQL Dump
-- version 4.6.6deb5
-- https://www.phpmyadmin.net/
--
-- Host: localhost:3306
-- Generation Time: Oct 25, 2018 at 11:49 PM
-- Server version: 5.7.23-0ubuntu0.18.04.1
-- PHP Version: 7.2.10-0ubuntu0.18.04.1

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";

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
  `id` int(4) UNSIGNED NOT NULL,
  `name` varchar(255) NOT NULL,
  `passwd` varchar(255) NOT NULL,
  `createdAt` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `lastSignIn` datetime NOT NULL
) ENGINE=InnoDB;

ALTER TABLE `users`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `name` (`name`);

--
-- Table structure for table `snippets`
--

CREATE TABLE `snippets` (
  `id` bigint(8) UNSIGNED NOT NULL,
  `digest` varchar(32) NOT NULL,
  `userId` int(4) UNSIGNED NOT NULL,
  `section` varchar(32) NOT NULL,
  `title` varchar(32) NOT NULL,
  `gitlabId` int(4) UNSIGNED NOT NULL
) ENGINE=InnoDB;

ALTER TABLE `snippets`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `digest` (`digest`),
  ADD KEY `userId` (`userId`);

--
-- AUTO_INCREMENT for tables
--
ALTER TABLE `users`
  MODIFY `id` int(4) UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=1;

ALTER TABLE `snippets`
  MODIFY `id` bigint(8) UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=1;
