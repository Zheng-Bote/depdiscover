# depdiscover Documentation

Welcome to the documentation for **depdiscover**, a native C++ Dependency Scanner and SBOM (Software Bill of Materials) Generator.

## Core Documentation

- [Architecture Overview](architecture/architecture.md) - Detailed information about the system design, components, and data flow.

## Screenshots and Reports

### Overview of the HTML Report
The interactive HTML report provides a comprehensive overview of all project dependencies, including their versions, licenses, and security status.

![Overview HTML Report](img/overview_html-report.png)

### Vulnerability Details
When vulnerabilities are detected, the report displays detailed information from OSV.dev, including CVSS scores and fixed versions.

![HTML Report Details](img/html_report.png)

### Vulnerability Suppression
The tool allows for manual suppression of specific CVEs with a provided reason, which is reflected in the final report.

![Suppressing Vulnerabilities](img/suppressing_html-report.png)

### Build Breaker (Fail on CVSS)
Integrate depdiscover into your CI/CD pipeline. You can configure the tool to fail the build if vulnerabilities above a certain CVSS threshold are found.

![Fail on CVSS](img/fail-on-cvss.png)

---
*Created by ZHENG Robert. Copyright (c) 2026.*
