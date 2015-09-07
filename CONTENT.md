Content Extraction Details
==========================


Word Extraction as Text
-----------------------
There are two formats available for extracted words, .txt and .json. They are enabled via the `-text` and `-json` options it will output all the words found each page.

The .txt output will dump all words on individual lines with no additional formatting to a file called `page_%04d_words.txt`.

Example:

```
where
the
second
inequality
follows
by
our
choice
of
M
.
```


Word Extraction as JSON
-----------------------
JSON based word extraction will also output the position of the word on the page. The format of the JSON file is:

```javascript
[ {"position":[0.26669,0.71900,0.30952,0.73084],"text":"where"},
  {"position":[0.31483,0.71900,0.33808,0.73084],"text":"the"},
  {"position":[0.34339,0.71900,0.39184,0.73084],"text":"second"},
  {"position":[0.39717,0.71900,0.46924,0.73084],"text":"inequality"},
  {"position":[0.47459,0.71900,0.52395,0.73084],"text":"follows"},
  {"position":[0.52931,0.71900,0.54699,0.73084],"text":"by"},
  {"position":[0.55230,0.71900,0.57653,0.73084],"text":"our"},
  {"position":[0.58186,0.71900,0.62602,0.73084],"text":"choice"},
  {"position":[0.63134,0.71900,0.64483,0.73084],"text":"of"},
  {"position":[0.65012,0.71900,0.66636,0.73084],"text":"M"},
  {"position":[0.66819,0.71900,0.67285,0.73084],"text":"."}
]
```

The filename for each page will be `page_%04d_words.json`

The `position` field denotes the four box coordinates as absolute values between 0 and 1 which are relative to the top left. A region covering the whole page would be `[0,0,1,1]`, one covering just the bottom right quater of the page would be `[0.5,0.5,1.0,1.0]`.

These represent the coordinates:

    [x1, y1, x2, y2].

To convert between relative positions and absolute pixel coordinates of each point of the region we multiply the dimension size (in pixels) by the position.

Example, for a page of 768x1024 and [0.5,0.5,1.0,1.0]:

    x1: 0.5*768 = 384
    y1: 0.5*1024 = 512
    x2: 1.0*768 = 768
    y2: 1.0*1024 = 1024

Graphical Representation of Coordinates:

```
       y1
       |
       |
       v
x1---->+-----------+
       |           |
       |           |
       |           |
       |           |
       |           |
       |           |
       +-----------+<----x2
                   ^
                   |
                   |
                   y2
```

The width and height of the region can be calculated as such:

```
  width = x2 - x1

  height = y2 - y1
```


Region Extraction as JSON
-------------------------
PandaPDF supports a limited subset of PDF actions, they are:

 * GoTo - Link to a Page within the same PDF (PDF 1.1)
 * GoToR - Link to a Page from another PDF (PDF 1.1)
 * URI - External link to a webpage (PDF 1.1)
 * Named - Link to a Named destination (beta, PDF 1.2)

For more information see PDF v1.2 §6.9 (pg98 - Actions).

Region extraction is enabled by specifying `-regions` on the commandline, they will be dumped into a file named `page_%04d_regions.json` for each page.

There is a translation between regions found in the PDF file and the outputted JSON regions in order to keep a consistent format. Each region has the following keys:

 * x1
 * y1
 * x2
 * y2
 * region_type
 * title
 * url

Example Regions File:

```javascript
[ {"x1":0.19445,"y1":0.03596,"x2":0.26429,"y2":0.04693,
   "region_type":"link","title":"","url":"http://pixel-mags.com/"}
]
```


GoTo Regions
++++++++++++
These regions navigate to another page within the same issue, it supports both named and exact page destinations.

All GoTo regions rely on the PixelMags Issue ID to be passed as a commandline argument using `-pm-issue-id`.

A GoTo region for 'Page 8' would be translated into:

```javascript
  region_type: link
  url: pm-page://local/$ISSUEID/8
```

A named destination of 'MyArticle' would be translated into:

```javascript
  region_type: link
  url: pm-page://local/$ISSUEID/MyArticle
```

Both Named and Exact Page destinations can be combined.


GoToR Regions
+++++++++++++
The difference between GoTo and GoToR regions is that instead of an Issue ID it uses the filename.

A remote link for 'Page 8' of example.pdf would be translated into:

```javascript
  region_type: link
  url: pm-page://local/example.pdf#8
```


URI Regions
+++++++++++
The most common type of region is a standard hyperlink, usually to a web page.

A URI region for 'http://google.com/' would be translated to:

```javascript
  region_type: link
  uri: http://google.com/
```


Named Regions
+++++++++++++
Please note that while Named regions are supported by PandaPDF they are not not thoroughly tested, nor are they frequently used by PDFs.

Output format:

```javascript
  region_type: link
  url: named://$NAME
```
