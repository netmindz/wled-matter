# Copilot Guidance

## Project Context
This repository contains an ESPHome custom component for WLED Matter integration.

## Change Validation Requirement
After making any code or configuration change, you must verify the change by running:

```bash
esphome compile wled-matter.yaml
```

Do not consider a change complete until this command has been run and the result checked.

## Notes
- Prefer minimal, targeted edits.
- Keep compatibility with current ESPHome APIs.
- If build errors appear, fix them before finalizing.

