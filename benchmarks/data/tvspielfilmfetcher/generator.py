#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2023 Plata Hill <plata.hill@kdemail.net>
# SPDX-License-Identifier: LGPL-2.1-or-later

begin = """<!--
  - SPDX-License-Identifier: CC0-1.0
  - SPDX-FileCopyrightText: 2023 Plata Hill <plata.hill@kdemail.net>
 -->
<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml" lang="de-DE">
<body>
<table class="info-table">"""

rowTemplate = """
    <tr class="hover">
        <td class="programm-col1">
            <a href="https://www.tvspielfilm.de/tv-programm/sendungen/swrsr,SWR.html" title="SWR/SR Programm">
                    <span class="logotype">
                        <picture>
                            <source srcset="https://a2.tvspielfilm.de/images/tv/sender/mini/swr.webp" type="image/webp">
                            <source srcset="https://a2.tvspielfilm.de/images/tv/sender/mini/swr.png" type="image/png">
                            <img src="https://a2.tvspielfilm.de/images/tv/sender/mini/swr.png" alt="SWR/SR Programm"
                                 title="SWR/SR Programm" loading="eager" width="40" height="40">
                        </picture>
                     </span>
            </a></td>
        <td class="col-2">
            <div>
                <strong>{startTime} - {endTime}</strong>
                <span>Mi 28.12.</span>
            </div>
        </td>
        <td class="col-3">
            <span>
                <a href="https://www.tvspielfilm.de/tv-programm/sendung/description.html"
                   title="Title">
                    <strong>Title</strong>
                </a>
            </span>
            <div id="progressbar_1" class="progressbar" style="display:none;">
                <div id="progressbar_1"></div>
                <span id="pinfo_1" class="progressbar-info" data-rel-start="{startTimeEpoch}" data-rel-end="{endTimeEpoch}"></span>
            </div>
        </td>
        <td class="col-4">
            <span>Category</span>
        </td>
        <td class="col-5">
            <span></span>
        </td>
        <td class="col-6"><span class="editorial-rating small"></span></td>
    </tr>"""

end = """
</table>
</body>
</html>
"""

with open('swr.html', 'w') as f:
    f.write(begin)

    num_minutes = 60 * 24 - 1
    for minute in range(num_minutes):
        f.write(rowTemplate.format(
            startTime = "{:02d}:{:02d}".format(int(minute / 60), minute % 60),
            endTime = "{:02d}:{:02d}".format(int((minute + 1) / 60), (minute + 1) % 60),
            startTimeEpoch = 1672182000 + minute * 60,
            endTimeEpoch = 1672182000 + (minute + 1) * 60
        ))
    f.write(end)
