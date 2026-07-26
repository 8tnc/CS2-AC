<div align="center">

<img src="docs/cs2ac-logo.png" width="760" alt="CS2AC">

### Open-source server-side anti-cheat for Counter-Strike 2.

[![Build](https://img.shields.io/github/actions/workflow/status/karola3vax/CS2AC/build.yml?branch=main&style=for-the-badge&label=build)](https://github.com/karola3vax/CS2AC/actions/workflows/build.yml)
[![Version](https://img.shields.io/badge/version-1.0.0-blue?style=for-the-badge)](https://github.com/karola3vax/CS2AC)
[![Detections](https://img.shields.io/badge/detections-17-red?style=for-the-badge)](#17-detections-one-plugin)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-5c7cfa?style=for-the-badge)](#quickstart)
[![License](https://img.shields.io/badge/license-AGPL--3.0-2ea44f?style=for-the-badge)](LICENSE)

**Counter-Strike is at its best when every shot, clutch, and win is earned.**

CS2AC helps community servers keep it that way.

Server-side only. Nothing for players to install. Just join and play.

[Download](https://github.com/karola3vax/CS2AC/releases) · [Install](#quickstart) · [See every detection](#17-detections-one-plugin) · [Pair it with CS2FOW](#want-wallhack-protection-too)

</div>

Some cheaters rage. Others smooth the aim and try to look legit. CS2AC looks for both in the aim, shots, movement, and inputs sent to the server.

<table>
<tr>
<td width="50%"><strong>Catches more than spinbots</strong><br>Snap aim, smooth aimlock, silent aim, anti-aim, bhop scripts, rapid fire, and more.</td>
<td width="50%"><strong>Nothing to install</strong><br>No client or kernel driver. Join the server and play like normal.</td>
</tr>
<tr>
<td width="50%"><strong>The whole server sees it</strong><br>When someone is caught, the detection appears in chat and at the center of the screen.</td>
<td width="50%"><strong>Evidence before punishment</strong><br>Every detection comes from a specific cheating pattern, not a guess based on K/D.</td>
</tr>
</table>

## See it catch

Real in-game clips. No mockups.

<table>
<tr>
<td width="50%" align="center">
<img src="docs/showcase/aimbot.gif" width="100%" alt="CS2AC detecting a blatant snap-hit aimbot">
<br><strong>AIMBOT</strong><br>
<sub>A blatant snap lands on target. CS2AC keeps the evidence.</sub>
</td>
<td width="50%" align="center">
<img src="docs/showcase/aimlock.gif" width="100%" alt="CS2AC detecting inhumanly precise target tracking">
<br><strong>AIMLOCK</strong><br>
<sub>The crosshair follows a moving target with inhuman precision.</sub>
</td>
</tr>
<tr>
<td width="50%" align="center">
<img src="docs/showcase/antiaim.gif" width="100%" alt="CS2AC detecting impossible anti-aim angles">
<br><strong>ANTIAIM</strong><br>
<sub>Impossible angles, attack-return, jitter, and spin patterns.</sub>
</td>
<td width="50%" align="center">
<img src="docs/showcase/bhop.gif" width="100%" alt="CS2AC detecting automated bunny hopping">
<br><strong>BHOP</strong><br>
<sub>Repeated frame-perfect hops and machine-like jump patterns.</sub>
</td>
</tr>
<tr>
<td width="50%" align="center">
<img src="docs/showcase/irregular-behavior.gif" width="100%" alt="CS2AC detecting repeated irregular airborne and no-scope results">
<br><strong>IRREGULAR BEHAVIOR</strong><br>
<sub>Too many rage-level airborne and no-scope results.</sub>
</td>
<td width="50%" align="center">
<img src="docs/showcase/silentaim.gif" width="100%" alt="CS2AC detecting bullets that disagree with visible aim">
<br><strong>SILENTAIM</strong><br>
<sub>The bullets hit somewhere the visible aim never pointed.</sub>
</td>
</tr>
</table>

## 17 detections. One plugin.

### Aim and accuracy

- **Aimbot** — Detects blatant snap-to-target aim. The cheat makes a sudden correction just before firing.<br>**What CS2AC sees:** the exact aim command behind a damaging shot moved sharply onto the victim, with the same behavior repeated across several hits.
- **Aimlock** — Detects smooth tracking that is too clean to be human. The cheat keeps the crosshair glued to a moving enemy without needing to snap or shoot.<br>**What CS2AC sees:** two seconds of near-perfect following while the target is moving, even through walls, repeated across separate tracking episodes.
- **Silentaim** — Detects bullets that do not agree with the player's visible aim. The cheat sends the shot in another direction while making the crosshair look normal.<br>**What CS2AC sees:** one confirmed weapon fire, bullet impact, and damaging hit whose direction is far away from the exact firing angle.
- **Inhuman Accuracy** — Detects rage-level accuracy from strong aim assistance or no-spread. The cheat removes the misses that even great players still make.<br>**What CS2AC sees:** at least 20 hits from 24 qualifying shots aimed at one clear enemy from 100 units or farther.
- **Irregular Behavior** — Detects repeated airborne and no-scope results associated with rage cheating and no-spread.<br>**What CS2AC sees:** every difficult attempt, hit or miss, along with distance and headshots; one lucky kill is never enough.

### Movement

- **Autostrafe** — Detects cheats that steer through the air automatically to preserve or gain speed.<br>**What CS2AC sees:** recent jumps repeatedly contain impossible strafe speed, efficiency, or perfectly optimized direction changes.
- **Bhop** — Detects scripts that press jump at the exact moment the player touches the ground.<br>**What CS2AC sees:** a long chain of frame-perfect hops or the same tiny landing pattern repeated again and again, with teleports, noclip, and bad collisions ignored.
- **Hyperscroll** — Detects cheats that flood the game with far more jump inputs than normal mouse-wheel scrolling.<br>**What CS2AC sees:** a high number of jump presses around each landing combined with an unusually high rate of frame-perfect hops across a full sample.
- **Nulls** — Detects movement binds that switch opposite keys with automated precision while airborne.<br>**What CS2AC sees:** the exact press-and-release timing, air speed, and player frame rate form a long chain of perfect direction switches.

### Exploits and client behavior

- **Antiaim** — Detects cheats that send fake or impossible view angles to confuse other players and systems.<br>**What CS2AC sees:** invalid pitch or roll, sustained spinning, repeated jitter, conflicting aim records, or an angle that snaps away for a shot and immediately returns.
- **DLL Injection** — Detects injected cheats that listen to hidden game events to run their features.<br>**What CS2AC sees:** the player's client subscribes to a blacklisted event that normal play does not need; it is checked after joining and again during the match.
- **Desubticking** — Detects cheats that strip normal between-tick timing from movement inputs.<br>**What CS2AC sees:** almost every timed input arrives at the same zero point across a sustained sample, rather than one unusual command.
- **Doubletap** — Detects the tick-shifting exploit that makes one weapon fire twice before it is ready.<br>**What CS2AC sees:** two ballistic shots from the same active weapon arrive closer together than that weapon's real firing cycle allows.
- **Invalid CVar** — Detects protected or safety-critical client settings changed to values normal play should never use.<br>**What CS2AC sees:** live checks of settings such as `m_yaw`, `fps_max`, sensitivity, view limits, and cheat-protected options return an invalid value.
- **Invalid Input** — Detects cheats that change movement buttons without sending the input history CS2 normally creates.<br>**What CS2AC sees:** repeated commands say a button changed, but their own press-and-release records do not contain that change.
- **Namechanger** — Detects rapid name spam used to distract players or show off a cheat feature.<br>**What CS2AC sees:** the same player changes their visible name five times within one rolling minute.
- **Subtick Spam** — Detects aliases that pack repeated movement and angle changes into the same moment.<br>**What CS2AC sees:** many commands contain matching same-time presses and releases carrying aim changes within a very short window.

## One detection. Everywhere.

When CS2AC acts, it can do all of this at once:

1. Announce the detection in public chat.
2. Hold a clear center-screen alert for five seconds.
3. Write the evidence and punishment result to the server console.
4. Run your configured ban or kick command.
5. Send a detailed Discord webhook report.

```text
[CS2AC] detected AIMBOT on Player and punished.
```

<div align="center">

<img src="docs/showcase/announcement-chat.png" width="600" alt="CS2AC test announcement in public chat">

<table>
<tr>
<td width="33%" align="center">
<img src="docs/showcase/announcement-center.png" width="100%" alt="CS2AC center-screen test announcement">
<br><strong>Five-second center alert</strong>
</td>
<td width="33%" align="center">
<img src="docs/showcase/detection.png" width="100%" alt="CS2AC center-screen Aimbot detection">
<br><strong>Detection sent</strong>
</td>
<td width="33%" align="center">
<img src="docs/showcase/whitelist.png" width="100%" alt="CS2AC announcing a detection on a whitelisted player">
<br><strong>Whitelist stays visible</strong>
</td>
</tr>
</table>

</div>

Whitelisting does not silence CS2AC. The detection still appears in chat, on screen, in the console, and in Discord; only the punishment command is skipped.

## Quickstart

You need a Windows x64 or Linux x64 CS2 dedicated server running [Metamod:Source](https://www.sourcemm.net/) 2.x.

1. Download the matching package from [GitHub Releases](https://github.com/karola3vax/CS2AC/releases).
2. Extract it into the CS2 server root without rearranging anything. The package begins with the `game` folder.
3. Edit `game/csgo/cfg/cs2ac.cfg`.
4. Start the server.
5. Run `meta list`, then `cs2ac_status`.

That is it. Players install nothing.

The default punishment commands are made for [CS2-SimpleAdmin](https://github.com/daffyyyy/CS2-SimpleAdmin). Using another admin plugin? Replace the two commands in `cs2ac.cfg` with commands that plugin understands.

## Configuration that makes sense

The included [`cs2ac.cfg`](cfg/cs2ac.cfg) explains every option in plain language.

| Setting | Default | What it does |
| --- | ---: | --- |
| `cs2ac_enabled` | `1` | Master switch for CS2AC. |
| `cs2ac_whitelist` | empty | SteamID64s that may be detected but must never be punished. |
| `cs2ac_*_enabled` | `1` | Enable or disable one detection module. |
| `cs2ac_chat_announcements` | `1` | Show detections in public chat. |
| `cs2ac_center_announcements` | `1` | Show the five-second center alert. |
| `cs2ac_punishment_command` | `css_addban ...` | Command used for permanent bans. |
| `cs2ac_kick_command` | `css_kick ...` | Command used for kick-only detections. |
| `cs2ac_webhook_url` | empty | Discord webhook that receives detection reports. |
| `cs2ac_webhook_role_id` | empty | Discord role to mention on a report. |
| `cs2ac_webhook_server_address` | automatic | Server address shown in Discord. |
| `cs2ac_allow_sv_cheats_testing` | `0` | Allow local detector testing with `sv_cheats 1`. Never enable this on a public server. |

Punishment commands support `{steamid64}`, `{userid}`, and `{detection}`:

```cfg
cs2ac_punishment_command "css_addban {steamid64} 0 CS2AC: {detection}"
cs2ac_kick_command "css_kick #{userid} CS2AC: {detection}"
```

Whitelist one account or a comma-separated list:

```cfg
cs2ac_whitelist "76561198000000001,76561198000000002"
```

### Discord in four steps

1. Create a webhook in the Discord channel that should receive detections.
2. Put its URL in `cs2ac_webhook_url`.
3. Run `cs2ac_reload`.
4. Run `cs2ac_webhook_test`.

Keep the webhook URL private. CS2AC never prints it back to the console.

<div align="center">
<img src="docs/showcase/webhook.png" width="432" alt="CS2AC Discord detection report with player, evidence, punishment, map, and server details">
</div>

<details>
<summary><strong>Server commands</strong></summary>

| Command | What it does |
| --- | --- |
| `cs2ac_status` | Show whether CS2AC and its main features are working. |
| `cs2ac_help` | List the available CS2AC commands. |
| `cs2ac_reload` | Reload `cs2ac.cfg`. |
| `cs2ac_check_config` | Find mistakes in the current configuration. |
| `cs2ac_test_announcement` | Preview the chat and center-screen alert without detecting anyone. |
| `cs2ac_webhook_test` | Send a test detection report to Discord. |

</details>

## Straight answers

<details>
<summary><strong>Do players install anything?</strong></summary>

No. CS2AC runs inside the dedicated server. Players connect and play normally.

</details>

<details>
<summary><strong>Does it work in Premier or Valve matchmaking?</strong></summary>

No. You need control of a community or dedicated server running Metamod:Source.

</details>

<details>
<summary><strong>Can it catch every cheat?</strong></summary>

No anti-cheat can promise that. CS2AC sees the cheating behavior that reaches the server; it cannot inspect a player's files, memory, or desktop.

That also means **DLL Injection does not scan anyone's PC**. It looks for suspicious game-event subscriptions that the client exposes to the server.

</details>

<details>
<summary><strong>Which detections ban and which only kick?</strong></summary>

Desubticking, Nulls, and Subtick Spam use the kick command by default. Every other detection uses the permanent-ban command by default.

You can replace or empty either command without disabling detection announcements.

</details>

<details>
<summary><strong>What happens to whitelisted players?</strong></summary>

They are still detected and shown everywhere, but CS2AC never sends a punishment command for them.

</details>

<details>
<summary><strong>Does it support FFA?</strong></summary>

Yes. CS2AC follows `mp_teammates_are_enemies`, so other players are treated as enemies when FFA is enabled.

</details>

<details>
<summary><strong>Does CS2AC advertise itself?</strong></summary>

Every six completed rounds, CS2AC shows one chat and center-screen credit:

```text
[CS2AC] This server is protected by karola3vax's anti-cheat.
```

This project credit is built in and has no disable option.

</details>

## Want wallhack protection too?

**CS2AC catches cheating behavior. [CS2FOW](https://github.com/karola3vax/CS2FOW) stops your server from sending live enemy positions through solid walls and smoke.**

They solve different problems, run entirely on the server, and can protect the same CS2 community server together.

## Building from source

Clone the pinned submodules:

```sh
git clone --recursive https://github.com/karola3vax/CS2AC.git
cd CS2AC
```

Windows needs Python 3.8 or newer and Visual Studio 2022 with the C++ workload:

```powershell
./build-windows.ps1
```

Linux needs Python 3.8 or newer and Docker:

```sh
./build-linux.sh
```

Both scripts make a directly installable package under the build folder's `package/game` directory. Linux builds inside the pinned Steam Runtime 3 SDK image.

## Be part of it

Run CS2AC on a real server. Test it. Break it. Send useful reports.

If it earns a place on your server, **star the repository and share your clips**. That is how an open-source anti-cheat gets harder to bypass.

## License

CS2AC is free and open-source software licensed under the [GNU Affero General Public License v3.0](LICENSE). Dependencies keep their own licenses; see [Third-party notices](THIRD_PARTY_NOTICES.md).
