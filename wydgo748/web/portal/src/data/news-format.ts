// News posts live in src/content/news/<slug>.md (DOCS/server/staff-operations.md).
// Format: a front-matter block of `key: value` lines between `---` markers,
// then plain-text paragraphs separated by blank lines. Markdown markup and
// HTML are not interpreted: every paragraph is rendered as escaped text.
export interface NewsPost {
  number: number;
  slug: string;
  date: string;
  category: string;
  title: string;
  summary: string;
  paragraphs: string[];
}

const keys = ['number', 'slug', 'date', 'category', 'title', 'summary'] as const;

export function parseNewsPost(file: string, raw: string): NewsPost {
  const text = raw.replace(/\r\n/g, '\n');
  const match = /^---\n([\s\S]*?)\n---\n([\s\S]*)$/.exec(text);
  if (!match) throw new Error(`${file}: missing front matter`);
  const fields = new Map<string, string>();
  for (const line of match[1].split('\n')) {
    const separator = line.indexOf(': ');
    const key = separator > 0 ? line.slice(0, separator) : '';
    if (!(keys as readonly string[]).includes(key) || fields.has(key)) throw new Error(`${file}: invalid front matter line "${line}"`);
    fields.set(key, line.slice(separator + 2).trim());
  }
  for (const key of keys) if (!fields.get(key)) throw new Error(`${file}: missing ${key}`);
  const number = Number(fields.get('number'));
  const post: NewsPost = {
    number, slug: fields.get('slug')!, date: fields.get('date')!, category: fields.get('category')!,
    title: fields.get('title')!, summary: fields.get('summary')!,
    paragraphs: match[2].split(/\n\s*\n/).map(p => p.replace(/\s*\n\s*/g, ' ').trim()).filter(Boolean),
  };
  if (!Number.isInteger(number) || number < 1) throw new Error(`${file}: number must be a positive integer`);
  if (!/^[a-z0-9]+(?:-[a-z0-9]+)*$/.test(post.slug) || !file.endsWith(`/${post.slug}.md`) && file !== `${post.slug}.md`) throw new Error(`${file}: slug must match the file name`);
  if (!/^\d{4}-\d{2}-\d{2}$/.test(post.date) || Number.isNaN(Date.parse(`${post.date}T00:00:00Z`))) throw new Error(`${file}: invalid date`);
  if (!post.paragraphs.length) throw new Error(`${file}: empty body`);
  if ([post.title, post.summary, ...post.paragraphs].some(value => /<[a-zA-Z/!]/.test(value))) throw new Error(`${file}: HTML is not allowed`);
  return post;
}

// Newest first: the highest number is the latest post.
export function orderNews(posts: NewsPost[]) {
  const seen = new Set<string>(), numbers = new Set<number>();
  for (const post of posts) {
    if (seen.has(post.slug) || numbers.has(post.number)) throw new Error(`duplicate news slug or number: ${post.slug} #${post.number}`);
    seen.add(post.slug); numbers.add(post.number);
  }
  return [...posts].sort((a, b) => b.number - a.number);
}
