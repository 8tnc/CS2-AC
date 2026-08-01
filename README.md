<div align="center">

<img src="docs/cs2ac-logo.png" width="760" alt="CS2AC">

### Open-source server-side anti-cheat for Counter-Strike 2.

[![Build](https://img.shields.io/github/actions/workflow/status/karola3vax/CS2AC/build.yml?branch=main&style=for-the-badge&label=build)](https://github.com/karola3vax/CS2AC/actions/workflows/build.yml)
[![Detections](https://img.shields.io/badge/detections-17-red?style=for-the-badge)](#detection-modules)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-5c7cfa?style=for-the-badge)](#quickstart)
[![License](https://img.shields.io/badge/license-AGPL--3.0-2ea44f?style=for-the-badge)](LICENSE)

**A good Counter-Strike match should be decided by the players, not by who brought the better cheat.**

CS2AC gives community servers a server-side way to catch cheating behavior. It watches the aim, shots, movement, inputs, and settings that players send to the server, then acts when that evidence crosses a strict detection gate.

[Watch it work](#showcase) · [Read the detection modules](#detection-modules) · [Pair it with CS2FOW](#cs2ac-and-cs2fow)

</div>

## Showcase

<table>
<tr>
<td width="50%" align="center">
<img src="docs/showcase/aimbot.gif" width="100%" alt="CS2AC detecting a blatant snap-hit aimbot">
<br><strong>AIMBOT</strong><br>
<sub>A damaging shot follows a blatant snap onto the target.</sub>
</td>
<td width="50%" align="center">
<img src="docs/showcase/aimlock.gif" width="100%" alt="CS2AC detecting inhumanly precise target tracking">
<br><strong>AIMLOCK</strong><br>
<sub>The aim follows a moving enemy with machine-like precision.</sub>
</td>
</tr>
<tr>
<td width="50%" align="center">
<img src="docs/showcase/antiaim.gif" width="100%" alt="CS2AC detecting impossible anti-aim angles">
<br><strong>ANTIAIM</strong><br>
<sub>The view produces impossible angles, spin, jitter, or attack-return patterns.</sub>
</td>
<td width="50%" align="center">
<img src="docs/showcase/bhop.gif" width="100%" alt="CS2AC detecting automated bunny hopping">
<br><strong>BHOP</strong><br>
<sub>Landings and jump inputs repeat with automated timing.</sub>
</td>
</tr>
<tr>
<td width="50%" align="center">
<img src="docs/showcase/irregular-behavior.gif" width="100%" alt="CS2AC detecting repeated irregular airborne and no-scope results">
<br><strong>IRREGULAR BEHAVIOR</strong><br>
<sub>Rage-level airborne and no-scope results keep adding up.</sub>
</td>
<td width="50%" align="center">
<img src="docs/showcase/silentaim.gif" width="100%" alt="CS2AC detecting a firing angle that disagrees with visible aim">
<br><strong>SILENTAIM</strong><br>
<sub>The damaging firing angle does not follow the visible aim path.</sub>
</td>
</tr>
</table>

### CS2AC and CS2FOW

<div align="center">

<a href="https://github.com/karola3vax/CS2FOW">
<img src="docs/showcase/cs2fow-wallhack.gif" width="100%" alt="CS2FOW operating across Dust II long sightlines">
</a>

**CS2AC catches cheating behavior. CS2FOW stops hidden enemy positions from reaching the wallhack.**

<sub>They solve different problems, run entirely on the server, and can protect the same CS2 community server together.</sub>

</div>

## Detection output

When a detector reaches its gate, CS2AC can report the same result wherever the server owner needs it:

1. Announce it in public chat.
2. Hold a center-screen alert for five seconds.
3. Write the detector evidence to the server console.
4. Submit the configured ban or kick command to the server.
5. Send a detailed Discord webhook report.

Chat and center-screen announcements can be turned on or off independently. Submitting a punishment command means CS2AC handed it to the server; the installed admin plugin is responsible for understanding and carrying out that command.

```text
[CS2AC] detected AIMBOT on Player and punished.
```

<div align="center">

<img src="docs/showcase/announcement-chat.png" width="600" alt="CS2AC test announcement in public chat">

<table>
<tr>
<td width="33%" align="center">
<img src="docs/showcase/announcement-center.png" width="100%" alt="CS2AC center-screen test announcement">
<br><strong>Center alert</strong>
</td>
<td width="33%" align="center">
<img src="docs/showcase/detection.png" width="100%" alt="CS2AC center-screen Aimbot detection">
<br><strong>Detection announced</strong>
</td>
<td width="33%" align="center">
<img src="docs/showcase/whitelist.png" width="100%" alt="CS2AC announcing a detection on a whitelisted player">
<br><strong>Whitelist visible</strong>
</td>
</tr>
</table>

<img src="docs/showcase/webhook.png" width="432" alt="CS2AC Discord detection report with player, evidence, punishment, map, and server details">

</div>

Whitelisted players can still be detected and reported, but CS2AC stops before submitting a punishment command. Disabled output channels stay quiet.

## Detection modules

All 17 modules are enabled by default and can be switched off individually. The gates below describe the current source; they are not configuration settings.

### Aim and accuracy

**Aimbot.** A blatant aimbot snaps onto an enemy as it fires a damaging shot. CS2AC reconstructs the aim and target positions around that shot, then checks how sharply the aim moved and how much closer it landed to the target.

*Main gate: three qualifying snap-hit incidents within five minutes; the target must be at least 100 units away and the snap is evaluated inside a half-second history window.*

**Aimlock.** Some aim assistance follows a moving enemy instead of snapping once. CS2AC checks whether the aim stays inside a distance-based player-width cone while the same target keeps moving, including through walls.

*Main gate: three qualifying episodes within five minutes; each episode requires two seconds of tracking, at least 95% on-target samples, at least 128 units of target travel, and at least 200 units of distance.*

**Silentaim.** Silent aim changes the damaging firing angle without moving the visible aim along the same path. CS2AC compares the exact firing angle with the adjacent command path and gives the weapon room for its real inaccuracy and spread.

*Main gate: 10 rolling points within five minutes. A suspicious grounded hit adds 2, an airborne hit adds 3, and a hit more than 22.5 degrees beyond its allowance adds 4; headshots add 1, wallbangs and smoke each add 2, no-scopes add 1, and a normal confirmed hit removes 2.*

**Inhuman Accuracy.** Nospread and rage settings can keep landing aimed shots at a rate normal play cannot hold over a large sample. CS2AC counts only qualifying shots aimed inside a narrow target cone at a real enemy and records whether they caused damage.

*Main gate: at least 40 qualifying shots within five minutes, at least 100 units away, with at least 90% of them hitting.*

**Irregular Behavior.** Rage cheats often turn airborne and unscoped sniper attempts into repeated kills. CS2AC counts both the difficult attempts and their results, with extra weight for combinations such as long-range, headshot, and wallbang kills.

*Main gate: 16 points within five minutes, at least three successful difficult shots, at least four attempts, and a success rate of at least 50%; kills below 10 metres are ignored.*

### Movement

**Autostrafe.** Automated strafing repeats air movement with speed, efficiency, and timing that a player cannot keep producing by hand. CS2AC checks completed jumps and also watches for a separate strafe-optimizer pattern in the aim movement.

*Main gate: 15 suspicious jumps in the latest 20, or five when at least one exceeds 30 strafes per second; the optimizer path separately triggers above 90% rolling evidence.*

**Bhop.** A bhop cheat keeps jumping on the landing frame or repeats the same small jump-input pattern. CS2AC evaluates recent landings while allowing failed jumps to weaken the pattern instead of pretending they never happened.

*Main gate: after at least 20 landing samples, either 10 consecutive frame-perfect hops or a decaying score of 7 with at least 90% of seven or more completed patterns repeating fewer than four inputs.*

**Hyperscroll.** Hyperscroll sends large, repeated bursts of jump inputs around each landing. CS2AC compares the number of presses with how often those landings still become frame-perfect jumps.

*Main gate: at least 20 completed patterns and 20 eligible landings, averaging at least 16 jump inputs while more than 60% of the landings are frame-perfect.*

**Nulls.** Null movement scripts switch between opposite directions with mechanically perfect release and press timing while airborne. CS2AC measures both movement axes, air speed, overlap, underlap, analog input, and the player's measured frame rate.

*Main gate: at least 128 input events and a dynamic perfect-timing score from 128 to 640 while moving at least 100 units per second in the air; lower frame rates and underlap require more evidence.*

### Client behavior

**Antiaim.** Anti-aim produces invalid pitch or roll, repeated angle-command inconsistencies, attack-return snaps, sustained spin, or repeating jitter. CS2AC combines short events into a decaying score while sustained spin and jitter must continue long enough to stand on their own.

*Main gate: 100 evidence points, a direction-consistent spin of 320–999 degrees per second for 15 seconds, a spin of at least 1,000 degrees per second for 10 seconds, or an exact repeating jitter pattern for 10 seconds.*

**DLL Injection.** Some injected clients expose themselves by subscribing to unusual game events while the player is connected. CS2AC checks the event subscriptions shared with the server against a curated list.

*Main gate: any subscription matching one of the 117 checked events; the first scan runs 10 seconds after joining and later scans run every two minutes.*

This detector does not scan a player's files or memory, and it cannot prove or catch every possible DLL injection. It reports this specific server-visible behavior.

**Desubticking.** Desubticking strips the normal between-tick timing from movement input. CS2AC measures how often commands containing subtick input arrive with that timing forced to zero.

*Main gate: at least 30 commands with subtick input inside 20 seconds, with at least 90% carrying zero timing; the first 10 seconds after joining are ignored.*

**Doubletap.** Doubletap makes the same ballistic weapon fire twice only zero or one server tick apart. CS2AC counts matching rapid-fire pairs and uses recent network measurements before allowing punishment.

*Main gate: two qualifying rapid-fire pairs. Bad network conditions do not hide the detection, but they veto its punishment command.*

**Invalid CVar.** A modified client can report a monitored setting outside the values accepted by a normal game. CS2AC requests those settings directly and checks finite values, protected states, and expected ranges.

*Main gate: one invalid monitored value announces once; that CVar must return to a valid value before the same condition can announce again.*

**Invalid Input.** Malformed or manipulated commands can claim a button state that disagrees with their recorded presses and releases. CS2AC compares both parts of each command instead of trusting either one alone.

*Main gate: eight invalid commands within five seconds.*

**Namechanger.** Name-change cheats repeatedly replace the player's visible name to create spam or confusion. CS2AC keeps a separate rolling history for every connected player.

*Main gate: five visible name changes within one minute.*

**Subtick Spam.** Subtick spam floods one tick with repeated same-time input aliases carrying angle changes. CS2AC counts only commands matching that specific input pattern.

*Main gate: 20 suspicious commands within half a second.*

## Quickstart

You need a Windows x64 or Linux x64 CS2 dedicated server running [Metamod:Source](https://www.sourcemm.net/) 2.x.

1. Open this repository's **Releases** tab and choose the matching Windows or Linux package.
2. Extract it into the CS2 server root without rearranging anything. The package begins with the `game` folder.
3. Edit `game/csgo/cfg/cs2ac.cfg`.
4. Start the server.
5. Run `meta list`, then `cs2ac_status`.

That is it. Players install nothing.

The default punishment commands are made for [CS2-SimpleAdmin](https://github.com/daffyyyy/CS2-SimpleAdmin). If your server uses another admin plugin, replace the two commands in `cs2ac.cfg` with commands that plugin understands.

CS2AC checks for stable updates after startup and every six hours. A verified update is prepared in the background and installed on the next full server restart. Existing settings are copied into the new configuration layout, and the previous configuration and plugin binary are kept as backups.

## Configuration

<details>
<summary><strong>Configuration reference</strong></summary>

The included [`cs2ac.cfg`](cfg/cs2ac.cfg) explains every option in plain language.

| Setting | Default | What it does |
| --- | ---: | --- |
| `cs2ac_enabled` | `1` | Master switch for CS2AC. |
| `cs2ac_whitelist` | empty | SteamID64s that may be detected but must never be punished. |
| `cs2ac_*_enabled` | `1` | Enable or disable one detection module. |
| `cs2ac_chat_announcements` | `1` | Show detections in public chat. |
| `cs2ac_center_announcements` | `1` | Show the five-second center alert. |
| `cs2ac_language` | `en` | Language used for public messages and Discord reports. |
| `cs2ac_punishment_command` | `css_addban ...` | Command submitted for permanent-ban detections. |
| `cs2ac_kick_command` | `css_kick ...` | Command submitted for kick-only detections. |
| `cs2ac_webhook_url` | empty | Discord webhook that receives detection reports. |
| `cs2ac_webhook_role_id` | empty | Discord role to mention on a report. |
| `cs2ac_webhook_server_address` | automatic | Server address shown in Discord. |
| `cs2ac_webhook_logo_url` | empty | Override the default CS2AC image shown in Discord reports. |
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

Set `cs2ac_language` to one of the bundled language codes, then run `cs2ac_reload`:

`ar`, `bg`, `cs`, `da`, `de`, `el`, `en`, `es-419`, `es-es`, `et`, `fi`, `fr`, `he`, `hr`, `hu`, `id`, `it`, `ja`, `ko`, `lt`, `lv`, `nl`, `no`, `pl`, `pt-br`, `pt-pt`, `ro`, `ru`, `sk`, `sr`, `sv`, `th`, `tr`, `uk`, `vi`, `zh-cn`, `zh-tw`.

</details>

<details>
<summary><strong>Discord webhooks</strong></summary>

1. Create a webhook in the Discord channel that should receive detections.
2. Put its URL in `cs2ac_webhook_url`.
3. Run `cs2ac_reload`.
4. Run `cs2ac_webhook_test`.

Keep the webhook URL private. CS2AC never prints it back to the console.

</details>

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

## FAQ

<details>
<summary><strong>Do players install anything?</strong></summary>

No. CS2AC runs on the dedicated server, so players connect and play as usual.

</details>

<details>
<summary><strong>Does it work in Premier or Valve matchmaking?</strong></summary>

No. CS2AC is made for community and dedicated servers you control. It cannot be added to Premier or Valve matchmaking.

</details>

<details>
<summary><strong>Can it catch every cheat?</strong></summary>

No anti-cheat catches everything. CS2AC can only judge the behavior that reaches the server; it does not read a player's files, memory, or desktop.

</details>

<details>
<summary><strong>Which detections ban and which only kick?</strong></summary>

By default, Desubticking, Nulls, and Subtick Spam use the kick command. Invalid CVar also uses it for conditions that should be corrected rather than permanently banned. Other detections use the permanent-ban command.

Server owners can change or empty either command. Detection announcements continue to work.

</details>

<details>
<summary><strong>What happens to whitelisted players?</strong></summary>

They can still trigger a detection and its enabled reports, but CS2AC does not submit a punishment command for them.

</details>

<details>
<summary><strong>Does it support FFA?</strong></summary>

Yes. When `mp_teammates_are_enemies` is enabled, CS2AC treats other players as enemies just like the game does.

</details>

<details>
<summary><strong>Does CS2AC advertise itself?</strong></summary>

Yes. Five seconds after a player fully joins, CS2AC privately shows this message to that player in chat and at the center of their screen. The center message stays for three seconds, and no one else sees it:

```text
[CS2AC] This server is protected by karola3vax's anti-cheat.
```

This small project credit is built in and cannot be turned off.

</details>

## Building from source

<details>
<summary><strong>Windows and Linux build instructions</strong></summary>

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

</details>

## Contributing

Run CS2AC on a real server. Test it, send reproducible reports, and include the detector evidence whenever something looks wrong.

If CS2AC earns a place on your server, star the repository and share your clips. That helps more server owners find it and gives the project better real-world feedback.

## License

CS2AC is free and open-source software licensed under the [GNU Affero General Public License v3.0](LICENSE). Dependencies keep their own licenses; see [Third-party notices](THIRD_PARTY_NOTICES.md).
