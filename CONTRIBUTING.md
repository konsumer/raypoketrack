# Contributing to This Project

Thank you for your interest in improving our open-source library! To ensure high code quality, security, and a maintainable codebase, all contributors must adhere to this contribution policy. See [our code of conduct](./CODE_OF_CONDUCT.md).

## 1. Code Contribution Workflow

### Licensing & Developer Certificate of Origin (DCO)
* **Open Source License:** By contributing, you agree that your submissions will be licensed under the project's open-source license.
* **Sign Your Work:** All commits must be signed (`git commit -s`) to certify that you have the legal right to submit the code under the Developer Certificate of Origin.

### Branching & Pull Requests
* **Target Branch:** Always branch from and target the `main` branch.
* **Atomic PRs:** Keep Pull Requests (PRs) highly focused. One PR should address exactly one bug fix, feature, or documentation update.
* **Clean History:** Squashing commits before merging is preferred. Avoid messy merge commits in your PR history.

## 2. Artificial Intelligence (AI) Policy & Guardrails

We welcome the efficient use of developer tools, including Generative AI assistants (e.g., Copilot, Claude, ChatGPT). However, maintaining a secure and reliable open-source library requires strict boundaries around AI-generated code.

### Absolute Human Ownership
* **The "Understand Everything" Rule:** You are 100% accountable for every line of code, documentation, or configuration committed under your name. If you cannot explain exactly how a snippet of AI-assisted code works, do not submit it.
* **No Unvetted Code:** AI-generated outputs often contain subtle hallucinations or outdated API calls. You must manually step through, refactor, and verify all AI suggestions.

### Code Quality & Technical Debt
* **Reject Bloat:** AI tools make it easy to write vast amounts of code quickly. We prioritize minimal, elegant, and highly readable solutions. Do not submit bloated, over-engineered AI architectures.
* **Test Isolation:** You must write independent, deterministic tests for your code. Never allow an AI to exclusively write the tests for its own generated logic.

### Intellectual Property & Security
* **No Closed Proprietary Code:** Ensure your AI tool does not inject plagiarized code or copy-protected snippets from non-open-source licenses.
* **Vulnerability Audits:** Always double-check package names and dependencies recommended by an LLM to prevent dependency confusion attacks or malicious package injections.

## 3. Testing & Code Style

### Code Quality Standards
* **Linting:** Your code must pass all local linter and formatter rules before submission. Run `make format` to verify.
* **Testing:** All new features or bug fixes must include corresponding unit or integration tests. Total test coverage must not decrease.

### PR Review Checklist
Before marking your PR as ready for review, verify that:
1. All automated CI/CD checks pass successfully.
2. The code conforms strictly to the project's architectural guidelines.
3. Any AI assistance used has been thoroughly vetted and tested by you.
