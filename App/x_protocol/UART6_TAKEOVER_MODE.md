# TPMesh UART6 Takeover Mode (V0.8 Supplement)

## Background
In some deployments, TPMesh AT configuration is performed by another software module on `UART6`.  
After that configuration flow completes, the system restarts the full DDC process (or whole device), and `x_protocol` should run directly without re-applying module AT configuration.

## Goals
- Allow external module to temporarily take over `UART6`.
- Ensure `x_protocol` stops using `UART6` before takeover.
- Support startup mode where `x_protocol` skips its own module configuration sequence (`AT`, `AT+ADDR`, `AT+CELL`, `AT+LP`).
- Keep runtime behavior deterministic during takeover window.
- Use one-way handover model: release, then restart module/process; no in-process reclaim.

## Architecture Update
Two configuration ownership modes are introduced:

- `TPMESH_MODULE_INIT_BY_X_PROTOCOL`
  - Existing behavior.
  - `x_protocol` runs module init sequence in `tpmesh_bridge_task`.
- `TPMESH_MODULE_INIT_BY_EXTERNAL`
  - New behavior.
  - `x_protocol` does not send configuration AT commands.
  - External module must guarantee module parameters are already valid before restart.

UART6 ownership state is now explicit:
- `Owned by x_protocol`: normal bridge/heartbeat AT traffic.
- `Released for external`: AT TX/RX path is disabled, UART6 peripheral is deinitialized from `x_protocol`.

## Runtime Handover
1. External module requests takeover via `tpmesh_request_uart6_takeover()`.
2. `x_protocol` blocks until UART6 can be safely released (for example wait for TX busy to clear), then returns success.
3. External module takes UART6 and performs configuration.
4. External module terminates/restarts current x_protocol runtime.
5. x_protocol starts again from normal init path.

Important:

- No in-process UART6 reclaim API is provided in this model.
- No persistent "taken over" state is maintained across runtime.
- Ownership transfer point is `tpmesh_request_uart6_takeover()` + runtime restart boundary.

## Integration Notes
- During takeover, DDC heartbeat/register loop pauses AT actions.
- Top-node Ethernet hook no longer intercepts frames while UART6 is released.
- If `TPMESH_MODULE_INIT_BY_EXTERNAL` is enabled, deployment must ensure at least:
  - address/cell/power mode are already configured,
  - required URC behavior (such as `+NNMI`) is available.
