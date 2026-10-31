// Focused page state tests. Rendering and real game transactions are separate gates.
import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { runInNewContext } from 'node:vm';

const html = readFileSync(new URL('../Resources/PremiumNell.html', import.meta.url), 'utf8');
const script = html.match(/<script>([\s\S]*?)<\/script>/)[1];

function fixture(online = true) {
  const nodes = new Map(), commands = [], handlers = {};
  const element = () => ({
    children: [], value: '', textContent: '', disabled: false, hidden: false, open: false,
    attributes: {}, handlers: {},
    append(...children) { this.children.push(...children); },
    replaceChildren(...children) { this.children = children; },
    setAttribute(name, value) { this.attributes[name] = value; },
    addEventListener(name, handler) { this.handlers[name] = handler; },
    showModal() { this.open = true; }, close() { this.open = false; },
    set innerHTML(_) { throw new Error('Catalog data must not become HTML'); },
  });
  const node = id => {
    if (!nodes.has(id)) nodes.set(id, element());
    return nodes.get(id);
  };
  const document = {
    documentElement: {}, getElementById: node, createElement: element,
    addEventListener(name, handler) { handlers[name] = handler; },
  };
  let receive;
  const window = online ? { chrome: { webview: {
    postMessage(message) { commands.push(message); },
    addEventListener(name, handler) { assert.equal(name, 'message'); receive = handler; },
  } } } : {};
  const context = { document, window };
  runInNewContext(script, context);
  const update = patch => receive({ data: { locale: 'EN', balance: 5000, pending: false, result: -1,
    items: [{ slot: 2, id: 400, name: 'Sword', price: 1000, quantity: 1, refinement: 7 }], ...patch } });
  const buy = () => node('items').children[0].children.at(-1).children[1];
  return { node, commands, update, buy, handlers, document, context };
}

test('hover tooltip distinguishes equal-name items with localized help', () => {
 const f=fixture();
 f.update({items:[{slot:0,id:1,name:'Divine Circle',price:1,details:['Reset Survival']},{slot:1,id:2,name:'Divine Circle',price:1,details:['Reset Trade']}]});
 const cards=f.node('items').children;
 assert.match(cards[0].title,/Reset Survival/);
 assert.match(cards[1].title,/Reset Trade/);
});

test('page shares complete locale keys, falls back to English and escapes catalog text', () => {
  const f = fixture();
  assert.deepEqual(f.commands, ['ready']);
  assert.equal(runInNewContext('JSON.stringify(Object.keys(translations.EN).sort()) === JSON.stringify(Object.keys(translations.PT).sort())', f.context), true);
  assert.equal(runInNewContext('translations.EN.results.length === 10 && translations.PT.results.length === 10', f.context), true);
  f.update({ locale: 'PT', items: [{ slot: 26, name: '<img src=x onerror=alert(1)>', price: 1234, quantity: 10, refinement: 9 }] });
  assert.equal(f.document.documentElement.lang, 'pt-BR');
  assert.equal(f.node('close').attributes['aria-label'], 'Fechar');
  assert.equal(f.node('close').attributes.title, 'Fechar');
  const card = f.node('items').children[0];
  assert.equal(card.children[1].textContent, '<img src=x onerror=alert(1)>');
  assert.equal(card.children[2].textContent, 'Quantidade: 10 · Refinação +9');
  f.buy().onclick();
  assert.match(f.node('confirmText').textContent, /1\.234 Donate/);
  assert.equal(f.node('confirmDetails').children[1].textContent, 'Quantidade: 10 · Refinação +9');
  f.node('confirm').onclick();
  assert.deepEqual(f.commands, ['ready', 'buy:26']);
  f.update({ locale: 'unknown' });
  assert.equal(f.document.documentElement.lang, 'en');
  assert.equal(f.node('balanceLabel').textContent, 'Donate balance');
  assert.equal(f.node('close').attributes['aria-label'], 'Close');
  assert.equal(f.node('close').attributes.title, 'Close');
});

test('confirmation latches immediately and only completion permits another purchase', () => {
  const f = fixture();
  f.update();
  f.buy().onclick();
  assert.equal(f.node('confirmation').open, true);
  f.node('confirm').onclick();
  f.node('confirm').onclick();
  f.buy().onclick();
  f.node('close').onclick();
  f.handlers.keydown({ key: 'Escape' });
  assert.deepEqual(f.commands, ['ready', 'buy:2']);
  assert.equal(f.buy().disabled, true);
  f.update({ pending: true });
  assert.equal(f.node('close').disabled, true);
  assert.equal(f.buy().disabled, true);
  f.update({ result: 0, balance: 4000 });
  assert.match(f.node('status').textContent, /Purchase completed/);
  assert.equal(f.node('balance').textContent, '4,000');
  assert.equal(f.buy().disabled, false);
  f.node('close').onclick();
  assert.deepEqual(f.commands, ['ready', 'buy:2', 'close']);
});

test('catalog changes invalidate confirmation and stale or expired shops cannot buy', () => {
  const f = fixture();
  f.update();
  f.buy().onclick();
  f.update({ result: 4 });
  assert.equal(f.node('confirmation').open, false);
  assert.equal(f.buy().disabled, true);
  f.node('confirm').onclick();
  assert.deepEqual(f.commands, ['ready']);
  f.update({ result: 3 });
  assert.equal(f.buy().disabled, true);
  f.update();
  assert.equal(f.buy().disabled, false);
});

test('insufficient balance, filtering, empty catalog and standalone preview', () => {
  const f = fixture();
  f.update({ balance: 999, result: 5 });
  assert.equal(f.buy().disabled, true);
  assert.match(f.node('status').textContent, /enough Donate/);
  f.node('search').value = 'not found';
  f.node('search').handlers.input();
  assert.equal(f.node('items').children.length, 0);
  assert.equal(f.node('empty').hidden, false);
  assert.equal(f.node('empty').textContent, 'No matching items.');
  f.update({ items: [] });
  assert.equal(f.node('empty').textContent, 'No items are available.');
  const offline = fixture(false);
  assert.match(offline.node('status').textContent, /inside the game/);
  assert.deepEqual(offline.commands, []);
});

test('catalog and confirmation preserve additions, special effect data and safe text in both locales', () => {
  const f = fixture();
  for (const locale of ['EN', 'PT']) {
    const details = ['Damage : 30 (60)', '<img src=x onerror=alert(1)>'];
    const effects = [{ code: 2, value: 30 }, { code: 43, value: 9 }, { code: 0, value: 0 }];
    f.update({ locale, items: [{ slot: 2, name: 'Sword', price: 1000, details, effects }] });
    const additions = f.node('items').children[0].children[3];
    assert.equal(additions.children[0].textContent, details[0]);
    assert.equal(additions.children[1].textContent, details[1]);
    const data = additions.children[2];
    assert.equal(data.children.length, 3);
    assert.equal(data.children[0].textContent, locale === 'PT' ? 'Dados configurados do item' : 'Configured item data');
    assert.equal(data.children[1].textContent, locale === 'PT' ? 'Efeito 2: 30' : 'Effect 2: 30');
    f.buy().onclick();
    const textTree = node => [node.textContent, ...node.children.map(textTree)];
    assert.deepEqual(textTree(f.node('confirmDetails').children[0]), textTree(additions));
    // A catalog update with the same name must replace the displayed effects.
    f.update({ locale, items: [{ slot: 2, name: 'Sword', price: 1000, details: ['Defense : 40'], effects: [] }] });
    assert.equal(f.node('items').children[0].children[3].children[0].textContent, 'Defense : 40');
    assert.equal(f.node('confirmation').open, false);
  }
});

test('known local rejection releases the page latch and reports an unsent purchase', () => {
  const f = fixture();
  for (const locale of ['EN', 'PT']) {
    f.update({ locale });
    f.buy().onclick();
    f.node('confirm').onclick();
    assert.equal(f.buy().disabled, true);
    f.update({ locale, result: 9 });
    assert.equal(f.buy().disabled, false);
    assert.equal(f.node('close').disabled, false);
    assert.match(f.node('status').textContent, locale === 'PT' ? /não foi enviada/ : /was not sent/);
  }
});

test('categories and six-item pagination retain authoritative slot identifiers', () => {
  const f = fixture();
  const items = Array.from({ length: 15 }, (_, i) => ({ slot: 26 - i, id: 400 + i, name: `Item ${i}`, price: 100, category: i < 8 ? 0 : i < 11 ? 1 : i < 14 ? 2 : 3 }));
  f.update({ items, account: '<script>account</script>' });
  assert.equal(f.node('login').textContent, 'Login: <script>account</script>');
  assert.equal(f.node('categories').children.length, 4);
  assert.equal(f.node('items').children.length, 6);
  assert.equal(f.node('pages').children.length, 2);
  f.node('pages').children[1].onclick();
  assert.equal(f.node('items').children.length, 2);
  f.buy().onclick(); f.node('confirm').onclick();
  assert.equal(f.commands.at(-1), 'buy:20');
  f.update({ items, result: 0 });
  f.node('categories').children[1].onclick();
  assert.equal(f.node('items').children.length, 3);
  assert.equal(f.node('pages').children.length, 1);
  f.buy().onclick(); f.node('confirm').onclick();
  assert.equal(f.commands.at(-1), 'buy:18');
  f.update({ items, result: 0 });
  f.node('categories').children[3].onclick();
  assert.equal(f.node('items').children.length, 1);
  assert.equal(f.node('items').children[0].children[1].textContent, 'Item 14');
});

test('search resets and catalog shrink clamps the current page; language buttons translate tabs', () => {
  const f = fixture();
  const items = Array.from({ length: 13 }, (_, slot) => ({ slot, name: `Item ${slot}`, price: 100 }));
  f.update({ items });
  f.node('pages').children[2].onclick();
  f.node('search').value = 'Item 0'; f.node('search').handlers.input();
  assert.equal(f.node('items').children.length, 1);
  assert.equal(f.node('pages').children[0].attributes['aria-current'], 'page');
  f.node('search').value = ''; f.node('search').handlers.input();
  f.node('pages').children[2].onclick();
  f.update({ items: items.slice(0, 2) });
  assert.equal(f.node('items').children.length, 2);
  f.node('languagePT').onclick();
  assert.equal(f.node('categories').children[0].textContent, 'Destaque');
  assert.equal(f.node('pages').attributes['aria-label'], 'Páginas');
  f.node('languageEN').onclick();
  assert.equal(f.node('categories').children[0].textContent, 'Featured');
});

test('item icons only use bounded numeric IDs on the official image endpoint', () => {
  const f = fixture();
  f.update();
  const icon = f.node('items').children[0].children[0].children[0];
  assert.equal(icon.src, 'https://wydmisc.raidhut.com/icons/global_prod/400.gif');
  assert.equal(icon.referrerPolicy, 'no-referrer');
  icon.onerror();
  assert.equal(f.node('items').children[0].children[0].textContent, '#400');
  f.update({ items: [{ slot: 2, id: '../checkout', name: 'Safe text', price: 100 }] });
  assert.equal(f.node('items').children[0].children[0].children[0].src, undefined);
});
