# CONTRIBUTING

Contributions are very welcome, both large and small.

Correct patches (see Patch Requirements) will generally be accepted.

This is a small project, so I don't want to get bogged down in too much formality, but in general I'm striving to follow Pieter Hintjens' Collective Code Construction Contract (C4) as used by the ZeroMQ project: https://rfc.zeromq.org/spec:42/C4/

## Patch Requirements

Maintainers and Contributors MUST have a Platform account and SHOULD use their real names or a well-known alias.

A patch SHOULD be a minimal and accurate answer to exactly one identified and agreed problem.

A patch MUST adhere to the code style guidelines of the project (see below)

A patch SHALL NOT include non-trivial code from other projects unless the Contributor is the original author of that code.

A patch MUST compile cleanly and pass project self-tests on at least the principal target platform.

A patch commit message SHOULD consist of a single short (less than 50 character) line summarizing the change, optionally followed by a blank line and then a more thorough description.

A "Correct Patch" is one that satisfies the above requirements.

### AI-assisted work

Using AI for contributions is acceptable, given the following:

- YOU review the output before sending a non-Draft PR. Do NOT request review
  until YOU have checked the AI-generated PR and verify the following:
- REMOVE verbosity and blathering from documentation, comments, PR description,
  commit message, etc. All resources, including names, should be CONCISE and
  CLEAR. They should contain USEFUL information and nothing more.
- REMOVE and DEDUPLICATE redundant code, tests, explanations, etc. Explicitness
  and clarity are GOOD, but verbosity, over-explanation, and redundancy is BAD.

## Coding Style

This project uses the Linux kernel coding style:

See https://www.kernel.org/doc/html/latest/process/coding-style.html

Rust code in this project uses `clippy` and `rustfmt`. Please run `cargo clippy` and `cargo fmt` on your code before submitting a patch.

Our CI jobs will also run these on your PR in case you forget.  
