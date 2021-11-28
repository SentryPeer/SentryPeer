# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]
  - Added `event_uuid`, `collected_method` (passive or responsive) and `created_by_node_id` columns to `honey` table
  - Added `libuuid` library requirement for `created_by_node_id` and `event_uuid` column of `honey` table

## [0.0.2] - 2021-11-24
  - Properly exit when `sentrypeer -h` is called
  - Correct string errors discovered when building RPMs on Fedora
  - RPM specfile
  - Man page
  - README.md additions

## [0.0.1] - 2021-11-23
  - Initial release
  - SentryPeer can listen on all interfaces for SIP probes/messages on UDP port 5060 and save them to its sqlite database (*sentrypeer.db*)

