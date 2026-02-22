<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**

- [resolver](#resolver)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# resolver

```bash
curl -X POST https://api.osv.dev/v1/query \
   -H "Content-Type: application/json" \
   -d '{
     "version": "3.24.0",
     "package": {
       "name": "liblouis",
       "ecosystem": "Debian"
     }
   }'
```

**json**

```json
{
  "name": "liblouis",
  "version": "3.24.0",
  "licenses": ["LGPL-2.1"],
  "cves": [
    {
      "id": "CVE-2023-26768",
      "severity": "HIGH",
      "summary": "Buffer Overflow in compileTranslationTable.c and lou_setDataPath"
    }
  ]
}
```

**wrong**

```url
https://nvd.nist.gov/vuln/detail/DEBIAN-CVE-2025-11187
```

**right**

```url
https://osv.dev/vulnerability/CVE-2025-11187
```

**options**

2. Using NVD (National Vulnerability Database)
   The NVD is the official US government database. Here you can search directly for the name and version (via a "Keyword-Search"):

   1 curl "https://services.nvd.nist.gov/rest/json/cves/2.0?keywordSearch=liblouis%203.24.0"
   (Note: The NVD-API is sometimes slow and throttles requests without an API key, but usually works for single queries.)

3. Using OSV (Open Source Vulnerabilities)
   If you already know the CVE ID (e.g., CVE-2023-26768), this is the fastest way to get all the details in JSON format:

1 curl https://api.osv.dev/v1/vulns/CVE-2023-26768

4. Using GitHub Advisory Database (via GraphQL)
   GitHub hosts many advisories. A simpler way without GraphQL is the tool gh (GitHub CLI), if installed:

1 gh api graphql -f query='
2 {
3 securityVulnerabilities(first: 10, package: "liblouis", ecosystem: ACTIONS) {
4 nodes {
5 advisory { summary cvss { score } }
6 }
7 }
8 }'
(Since `liblouis` is not a native NPM/PyPI/Cargo Lib, GitHub is often less precise than OSV or NVD here.)
