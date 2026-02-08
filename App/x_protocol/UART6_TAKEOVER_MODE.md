# TPMesh UART6 Takeover Mode (V0.8 Supplement)

## Background
In some deployments, TPMesh AT configuration is performed by another software module on `UART6`.  
After that configuration flow completes, the system restarts the full DDC process (or whole device), and `x_protocol` should run directly without re-applying module AT configuration.

## Goals
- Allow external module to temporarily take over `UART6`.
- Ensure `x_protocol` stops using `UART6` before takeover.
- Support startup mode where `x_protocol` skips its own module configuration sequence (`AT`, `AT+ADDR`, `AT+CELL`, `AT+LP`).
- Keep runtime behavior deterministic during takeover window.

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
2. Caller must check return value:
   - `0`: UART6 released, external module can start configuration.
   - non-zero: release failed (for example TX busy), external module must not use UART6 yet.
3. `x_protocol` releases UART6 (`tpmesh_at_release_uart6()`), and AT traffic becomes unavailable.
4. External module performs configuration on UART6.
5. External module restarts DDC service (or device), then `x_protocol` starts in selected mode.

Important:

- `tpmesh_request_uart6_takeover()` is no longer fire-and-forget. It is a deterministic ownership API and must be checked by integration code.
- `tpmesh_reclaim_uart6_for_tpmesh()` performs quiet UART6 re-acquire (no startup probe bytes/banner on UART line).

Optional reclaim API is provided for non-restart scenarios:
- `tpmesh_reclaim_uart6_for_tpmesh()`

## Integration Notes
- During takeover, DDC heartbeat/register loop pauses AT actions.
- Top-node Ethernet hook no longer intercepts frames while UART6 is released.
- If `TPMESH_MODULE_INIT_BY_EXTERNAL` is enabled, deployment must ensure at least:
  - address/cell/power mode are already configured,
  - required URC behavior (such as `+NNMI`) is available.
