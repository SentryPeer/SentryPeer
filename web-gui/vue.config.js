/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only  */
/* Copyright (c) 2021 - 2022 Gavin Henry <ghenry@sentrypeer.org> */
/*
   _____            _              _____
  / ____|          | |            |  __ \
 | (___   ___ _ __ | |_ _ __ _   _| |__) |__  ___ _ __
  \___ \ / _ \ '_ \| __| '__| | | |  ___/ _ \/ _ \ '__|
  ____) |  __/ | | | |_| |  | |_| | |  |  __/  __/ |
 |_____/ \___|_| |_|\__|_|   \__, |_|   \___|\___|_|
                              __/ |
                             |___/
*/
module.exports = {
  devServer: {
    port: 8081,
  },
  outputDir: "../src/http_static",
  indexPath: "../http_index_route.h",
}

const { GitRevisionPlugin } = require("git-revision-webpack-plugin")
const git = new GitRevisionPlugin({ versionCommand: "rev-parse --short HEAD" })

process.env.VUE_APP_COPYRIGHT = require("./package.json").copyright
process.env.VUE_APP_DESCRIPTION = require("./package.json").description
process.env.VUE_APP_VERSION = require("./package.json").version
process.env.VUE_APP_GIT_REV = git.version()
