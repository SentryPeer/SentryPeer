# Security Policies and Procedures

This document outlines security procedures and general policies for the **SentryPeer** project.

* [Reporting a Vulnerability](#reporting-a-vulnerability)
* [Disclosure Policy](#disclosure-policy)
* [Comments on this Policy](#comments-on-this-policy)

## Reporting a Vulnerability

The **SentryPeer** team and community take all security vulnerabilities seriously. Thank you for improving the security
of **SentryPeer**. We appreciate your efforts and responsible disclosure and will make every effort to acknowledge your
contributions.

Report security bugs by emailing the lead maintainer at <ghenry@sentrypeer.org> and include the word "**SECURITY**" in
the subject line. If possible, please use [Gavin's PGP key](./ghenry_at_sentrypeer_org_pgp_key.txt).

The lead maintainer will acknowledge your email within 48 hours, and will send a more detailed response within 48 hours
indicating the next steps in handling your report. After the initial reply to your report, the security team will
endeavor to keep you informed of the progress towards a fix and full announcement, and may ask for additional
information or guidance.

Please report security vulnerabilities in third-party libraries to the person or team maintaining them.

Please also check [Coverity Scan](https://scan.coverity.com/projects/sentrypeer-sentrypeer) results to see if it is a
known issue.

## Disclosure Policy

When disclosing vulnerabilities please do the following:

* Your name and affiliation (if any)
* Include scope of vulnerability. Let us know who could use this exploit
* Document steps to identify the vulnerability. It is important that we can reproduce your findings
* Show how to exploit vulnerability, give us an attack scenario

When the security team receives a security vulnerability report, they will assign it to a primary handler. This person
will coordinate the fix and release process, involving the following steps:

* Confirm the problem and determine the affected versions
* Audit code to find any potential similar problems
* Prepare fixes for all releases still under maintenance. These fixes will be released as fast as possible

## Comments on this Policy

If you have suggestions on how this process could be improved please submit a pull request.

## Security Acknowledgments Hall Of Fame

* None to date
