import os
import sys
sys.path.insert(0, os.path.abspath('..'))

project = "pyGuidos"
copyright = "European Union, 2026, Giovanni Caudullo, Peter Vogt"
author = "Giovanni Caudullo, Peter Vogt"
release = "2.3.2"

extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.napoleon",
    "sphinx.ext.viewcode",
    "sphinx.ext.githubpages",
]

templates_path = ["_templates"]
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]

html_theme = "pydata_sphinx_theme"
html_static_path = ["_static"]
html_css_files = ["custom.css"]
html_favicon = "https://forest.jrc.ec.europa.eu/static/forest/images/logos/eu/favicon.ico"
html_sidebars = {"index": []}
html_context = {
    "default_mode": "light",
}

html_theme_options = {
    "logo": {
        "text": "pyGuidos",
        "image_light": "https://forest.jrc.ec.europa.eu/static/forest/images/logos/eu/logo-ec--en.svg",
        "image_dark": "https://forest.jrc.ec.europa.eu/static/forest/images/logos/eu/logo-ec--en.svg",
    },
    "icon_links": [
        {
            "name": "GitLab",
            "url": "https://jrc-forest.pages.code.europa.eu/guidos/pyguidos",
            "icon": "fa-brands fa-gitlab",
        },
    ],
    "show_prev_next": True,
    "navbar_align": "left",    
    "footer_start": ["copyright"],
    "footer_end": ["theme-version"],
    "secondary_sidebar_items": ["page-toc", "edit-this-page"], 
}