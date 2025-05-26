import requests
import os.path
import sys

UCD_URL = "https://www.unicode.org/Public/UCD/latest/ucd/UnicodeData.txt"

def fetch_ucd_txt(output: str) -> bool:
    os.makedirs(os.path.dirname(output), exist_ok=True)
    try:
        with requests.get(UCD_URL, stream=True) as response:
            response.raise_for_status()
            with open(output, "wb") as file:
                for chunk in response.iter_content(chunk_size=8192):
                    file.write(chunk)
            return True
    except requests.RequestException as e:
        print(f"Error fetching UnicodeData.txt: {e}", file=sys.stderr)
    except IOError as e:
        print(f"Error writing to file: {e}", file=sys.stderr)
    return False

def in_bmp(codepoint: int) -> bool:
    return 0x0000 <= codepoint <= 0xFFFF

def byte_count_in_utf8(codepoint: int) -> int:
    if codepoint < 0x80:
        return 1
    elif codepoint < 0x800:
        return 2
    elif codepoint < 0x10000:
        return 3
    else:
        return 4

def codepoints_to_ranges(codepoints: list[int]) -> list[tuple[int, int]]:
    if not codepoints:
        return []
    ranges = []
    start = codepoints[0]

    for i in range(1, len(codepoints)):
        if codepoints[i] != codepoints[i - 1] + 1:
            end = codepoints[i - 1] + 1
            ranges.append([start, end])
            start = codepoints[i]

    ranges.append([start, codepoints[-1] + 1])

    return ranges

def format_ranges(ranges: list[tuple[int, int]]) -> str:
    return ','.join(f"{{U'\\U{start:08X}', U'\\U{end:08X}'}}" for start, end in ranges)

def main():
    ucd_path = os.path.join(sys.argv[1], "UnicodeData.txt")
    if not os.path.exists(ucd_path):
        if not fetch_ucd_txt(ucd_path):
            return

    bmp_codepoints: list[int] = []
    non_bmp_codepoints: list[int] = []
    utf8_codepoints: list[list[int]] = [[], [], [], []]
    just_codepoints: list[int] = []

    range_start_codepoint: int | None = None

    with open(ucd_path, "r", encoding="utf-8") as file:
        for line_content in file:
            line = line_content.strip()
            if not line: # Skip empty lines
                continue

            fields = line.split(';')
            codepoint_hex = fields[0]
            name = fields[1]

            codepoints_from_this_entry: list[int] = []

            if name.endswith(", First>"):
                # Start of a new potential range, abandon any previous unclosed one.
                range_start_codepoint = int(codepoint_hex, 16)
                # No codepoints added yet; wait for a Last marker.
            elif name.endswith(", Last>"):
                if range_start_codepoint is not None:
                    current_cp_val = int(codepoint_hex, 16)
                    if range_start_codepoint <= current_cp_val: # Ensure First <= Last
                        for cp_val in range(range_start_codepoint, current_cp_val + 1):
                            codepoints_from_this_entry.append(cp_val)
                    # else: malformed range (Last < First), ignore.
                    range_start_codepoint = None # Reset range tracker
                # else: A <..., Last> without a preceding <..., First> is ignored for range purposes.
            else: # Regular codepoint line
                # If a range was started but is now interrupted by a regular line, abandon that range.
                if range_start_codepoint is not None:
                    range_start_codepoint = None
                
                codepoints_from_this_entry.append(int(codepoint_hex, 16))

            for cp in codepoints_from_this_entry:
                # Filter out surrogate codepoints (U+D800 to U+DFFF)
                if 0xD800 <= cp <= 0xDFFF:
                    continue
                
                # Add to appropriate lists
                if in_bmp(cp):
                    bmp_codepoints.append(cp)
                else:
                    non_bmp_codepoints.append(cp)
                
                # Add to UTF-8 byte count lists (ensure cp is valid, non-surrogate)
                if 0 <= cp <= 0x10FFFF: 
                    utf8_idx = byte_count_in_utf8(cp) - 1
                    utf8_codepoints[utf8_idx].append(cp)
                
                just_codepoints.append(cp)

    # Sort all collected codepoint lists before generating ranges
    bmp_codepoints.sort()
    non_bmp_codepoints.sort()
    for i in range(len(utf8_codepoints)):
        utf8_codepoints[i].sort()
    just_codepoints.sort()

    bmp_ranges: list[tuple[int, int]] = codepoints_to_ranges(bmp_codepoints)
    non_bmp_ranges: list[tuple[int, int]] = codepoints_to_ranges(non_bmp_codepoints)
    utf8_ranges: list[list[tuple[int, int]]] = [
        codepoints_to_ranges(cps_list) for cps_list in utf8_codepoints
    ]
    just_ranges: list[tuple[int, int]] = codepoints_to_ranges(just_codepoints)

    output_dir = sys.argv[2]
    os.makedirs(output_dir, exist_ok=True)
    with open(os.path.join(output_dir, "bmp_ranges.inc"), "w") as f:
        f.write(format_ranges(bmp_ranges))
    with open(os.path.join(output_dir, "non_bmp_ranges.inc"), "w") as f:
        f.write(format_ranges(non_bmp_ranges))
    for i, ranges in enumerate(utf8_ranges):
        with open(os.path.join(output_dir, f"utf8_ranges_{i + 1}.inc"), "w") as f:
            f.write(format_ranges(utf8_ranges[i]))
    with open(os.path.join(output_dir, "just_ranges.inc"), "w") as f:
        f.write(format_ranges(just_ranges))


if __name__ == "__main__":
    main()
